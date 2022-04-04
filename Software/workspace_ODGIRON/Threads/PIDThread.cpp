#if 1
/*
 * PIDThread.cpp
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 */
#include "Threads.hpp"
#include <TipThermoModel.hpp>
#include "power.hpp"

static TickType_t powerPulseWaitUnit      = 25 * TICKS_100MS;      // 2.5 s
static TickType_t powerPulseDurationUnit  = (5 * TICKS_100MS) / 2; // 250 ms
TaskHandle_t      pidTaskNotification     = NULL;
uint32_t          currentTempTargetDegC   = 0; // Current temperature target in C
int32_t           powerSupplyWattageLimit = 0;
bool              heaterThermalRunaway    = false;

static int32_t getPIDResultX10Watts(int32_t tError);
static void    detectThermalRunaway(const int16_t currentTipTempInC, const int tError);
static void    setOutputx10WattsViaFilters(int32_t x10Watts);
static int32_t getX10WattageLimits();

/* StartPIDTask function */
void doPIDTask() {
  /*
   * We take the current tip temperature & evaluate the next step for the tip
   * control PWM.
   */
  setTipX10Watts(0); // disable the output at startup

  currentTempTargetDegC = 0; // Force start with no output (off). If in sleep / soldering this will
                             // be over-ridden rapidly
  pidTaskNotification    = xTaskGetCurrentTaskHandle();
  uint32_t PIDTempTarget = 0;
  // Pre-seed the adc filters
  for (int i = 0; i < 128; i++) {
    osDelay(5);
    TipThermoModel::getTipInC(true);
    getInputVoltageX10();
  }
  int32_t x10WattsOut = 0;

	for (;;) {
		x10WattsOut = 0;
		// This is a call to block this thread until the ADC does its samples
		if (ulTaskNotifyTake(pdTRUE, 2000)) {
			// Do the reading here to keep the temp calculations churning along
			uint32_t currentTipTempInC = TipThermoModel::getTipInC(true);//瞬时采集温度
			PIDTempTarget = currentTempTargetDegC + systemSettings.balanceTempOffset;	//目标温度
			//usb_printf("currentTipTempInC = %d  PIDTempTarget = %d \r\n", currentTipTempInC, PIDTempTarget);
			if (PIDTempTarget) {
				// Cap the max set point to 450C
				if (PIDTempTarget > (450)) {
					// Maximum allowed output
					PIDTempTarget = (450);
				}

				// Safety check that not aiming higher than current tip can measure
				if (PIDTempTarget > TipThermoModel::getTipMaxInC()) {
					PIDTempTarget = TipThermoModel::getTipMaxInC();
				}
				int32_t tError = PIDTempTarget - currentTipTempInC;

				detectThermalRunaway(currentTipTempInC, tError);

				tError = (float)tError /(100.0 / systemSettings.pidKp);
				x10WattsOut = getPIDResultX10Watts(tError);			///关键的一句
				//usb_printf("x10WattsOut = %d \r\n", x10WattsOut);
			} else {
				detectThermalRunaway(currentTipTempInC, 0);
			}
			// Secondary safety check to forcefully disable header when within ADC noise of top of ADC
			// 输入电压小于8V,或手柄温度大于70,或菜单页面停止加热时，也关闭PID输出, 7FFF = 32767
			if ( //!(tipState == TIP_HEATING || tipState == TIP_SLEEPING_TIME_OUT) ||
					getTipRawTemp(0) > (0x7FFF - 150) ||
					((getInputVoltageX10()/10) < 8) ||
					(getHandleTemperature(true)/10) > 70) {
				x10WattsOut = 0;
			}
			setOutputx10WattsViaFilters(x10WattsOut);	//通过过滤器
		} else {
			// ADC interrupt timeout
			setTipPWM(0, false);
		}
#ifdef DEBUG_UART_OUTPUT
    log_system_state(x10WattsOut);
#endif
	}
}

template <class T = int32_t> struct Integrator {
  T sum;

  T update(const T val, const int32_t inertia, const int32_t gain, const int32_t rate, const int32_t limit) {
    // Decay the old value. This is a simplified formula that still works with decent results
    // Ideally we would have used an exponential decay but the computational effort required
    // by exp function is just not justified here in respect to the outcome
	// 衰减旧值。这是一个简化的公式，仍然可以取得不错的结果
	// 理想情况下，我们会使用【指数衰减】功率曲线确实是指数衰减，但需要计算量
	// 通过 exp 函数在这里就结果而言是不合理的
    sum = (sum * (100 - (inertia / rate))) / 100;
    // Add the new value x integration interval ( 1 / rate)
    sum += (gain * val) / rate;

    // limit the output
    if (sum > limit)
      sum = limit;
    else if (sum < -limit)
      sum = -limit;

    return sum;
  }

  void set(T const val) { sum = val; }

  T get(bool positiveOnly = true) const { return (positiveOnly) ? ((sum > 0) ? sum : 0) : sum; }
};
int32_t getPIDResultX10Watts(int32_t setpointDelta) {
  static TickType_t          lastCall   = 0;
  static Integrator<int32_t> powerStore = {0};

  const int rate = 1000 / (xTaskGetTickCount() - lastCall);
  //int rate = 1000 / (xTaskGetTickCount() - lastCall);
  //usb_printf("rate = %d \r\n", rate);	// = 0

  lastCall       = xTaskGetTickCount();
  // Sandman note:
  // PID Challenge - we have a small thermal mass that we to want heat up as fast as possible but we don't
  // want to overshot excessively (if at all) the setpoint temperature. In the same time we have 'imprecise'
  // instant temperature measurements. The nature of temperature reading imprecision is not necessarily
  // related to the sensor (thermocouple) or DAQ system, that otherwise are fairly decent. The real issue	is
  // the thermal inertia. We basically read the temperature in the window between two heating sessions when
  // the output is off. However, the heater temperature does not dissipate instantly into the tip mass so
  // at any moment right after heating, the thermocouple would sense a temperature significantly higher than
  // moments later. We could use longer delays but that would slow the PID loop and that would lead to other
  // negative side effects. As a result, we can only rely on the I term but with a twist. Instead of a simple
  // integrator we are going to use a self decaying integrator that acts more like a dual I term / P term
  // rather than a plain I term. Depending on the circumstances, like when the delta temperature is large,
  // it acts more like a P term whereas on closing to setpoint it acts increasingly closer to a plain I term.
  // So in a sense, we have a bit of both.
  //																		 So there we go...

  // P = (Thermal Mass) x (Delta Temperature ) / 1sec, where thermal mass is in X10 J / °C and
  // delta temperature is in °C. The result is the power in X10 W needed to raise (or decrease!) the
  // tip temperature with (Delta Temperature ) °C in 1 second.
  // Note on powerStore. On update, if the value is provided in X10 (W) units then inertia shall be provided
  // in X10 (J / °C) units as well. Also, powerStore is updated with a gain of 2. Where this comes from: The actual
  // power CMOS is controlled by TIM3->CTR1 (that is software modulated - on/off - by TIM2-CTR4 interrupts). However,
  // TIM3->CTR1 is configured with a duty cycle of 50% so, in real, we get only 50% of the presumed power output
  // so we basically double the need (gain = 2) to get what we want.

  // 睡魔注意：
  // PID 挑战 - 我们有一个小的热质量，我们希望尽快升温，但我们不这样做
  //想要过度（如果有的话）设定点温度。同时我们有“不精确”
  //即时温度测量。温度读数不精确的性质不一定
  // 与传感器（热电偶）或 DAQ 系统有关，否则它们相当不错。真正的问题是
  // 热惯性。我们基本上在两个加热时段之间读取窗口中的温度，当
  // 输出关闭。然而，加热器温度不会立即消散到尖端质量中，因此
  // 在加热后的任何时刻，热电偶会感应到明显高于
  // 片刻之后。我们可以使用更长的延迟，但这会减慢 PID 循环并导致其他
  // 负面影响。结果，我们只能依靠 I 术语，但有一个转折点。而不是一个简单的
  // 积分器 我们将使用自衰减积分器，它的作用更像是对偶 I 项 / P 项
  // 而不是一个简单的 I 术语。视情况而定，例如当 delta 温度很大时，
  // 它的行为更像是一个 P 项，而在接近设定点时，它的行为越来越接近于一个普通的 I 项。
  // 所以从某种意义上说，我们两者都有。
  // 所以我们开始...

  // P = (热质量) x (Delta 温度) / 1 秒，其中热质量以 X10 J / °C 为单位，并且
  // 增量温度以°C 为单位。结果是需要 X10 W 的功率来提高（或降低！）
  // 1 秒内 (Delta Temperature ) °C 的尖端温度。
  // 注意powerStore。更新时，如果值以 X10 (W) 为单位提供，则应提供惯性
  // 也以 X10 (J / °C) 为单位。此外，powerStore 以 2 的增益更新。这来自：实际
  // 电源 CMOS 由 TIM3->CTR1 控制（即软件调制 - 开/关 - 由 TIM2-CTR4 中断）。然而，
  // TIM3->CTR1 配置了 50% 的占空比，因此，实际上，我们只得到了假定功率输出的 50%
  // 所以我们基本上需要加倍（增益 = 2）来获得我们想要的东西。

  int32_t x10WOut = powerStore.update(TIP_THERMAL_MASS * setpointDelta, // the required power
          TIP_THERMAL_MASS,                 // Inertia, smaller numbers increase dominance of the previous value
          2,                                // gain
          rate,                             // PID cycle frequency
          getX10WattageLimits());

  //usb_printf("rate = %d  x10WOut = %d \r\n", rate, x10WOut);
  return x10WOut;
}

void detectThermalRunaway(const int16_t currentTipTempInC, const int tError) {
  static uint16_t   tipTempCRunawayTemp   = 0;
  static TickType_t runawaylastChangeTime = 0;

  // Check for thermal runaway, where it has been x seconds with negligible (y) temp rise
  // While trying to actively heat

  // If we are more than 20C below the setpoint
  if ((tError > THERMAL_RUNAWAY_TEMP_C)) {

    // If we have heated up by more than 20C since last sample point, snapshot time and tip temp
    int16_t delta = (int16_t)currentTipTempInC - (int16_t)tipTempCRunawayTemp;
    if (delta > THERMAL_RUNAWAY_TEMP_C) {
      // We have heated up more than the threshold, reset the timer
      tipTempCRunawayTemp   = currentTipTempInC;
      runawaylastChangeTime = xTaskGetTickCount();
    } else {
      if ((xTaskGetTickCount() - runawaylastChangeTime) > (THERMAL_RUNAWAY_TIME_SEC * TICKS_SECOND)) {
        // It has taken too long to rise
        heaterThermalRunaway = true;
      }
    }
  } else {
    tipTempCRunawayTemp   = currentTipTempInC;
    runawaylastChangeTime = xTaskGetTickCount();
  }
}

int32_t getX10WattageLimits() {
  int32_t limit = availableW10(0);

  if (systemSettings.powerLimit && limit > (systemSettings.powerLimit * 10)) {
    limit = systemSettings.powerLimit * 10;
  }
  //if (powerSupplyWattageLimit && limit > powerSupplyWattageLimit * 10) {
  //  limit = powerSupplyWattageLimit * 10;
  //}
  return limit;
}

void setOutputx10WattsViaFilters(int32_t x10WattsOut) {
  static TickType_t lastPowerPulseStart = 0;
  static TickType_t lastPowerPulseEnd   = 0;
#ifdef SLEW_LIMIT
  static int32_t x10WattsOutLast = 0;
#endif

  // If the user turns on the option of using an occasional pulse to keep the power bank on
  if (systemSettings.KeepAwakePulse) {
    const TickType_t powerPulseWait = powerPulseWaitUnit * systemSettings.KeepAwakePulseWait;
    if (xTaskGetTickCount() - lastPowerPulseStart > powerPulseWait) {
      const TickType_t powerPulseDuration = powerPulseDurationUnit * systemSettings.KeepAwakePulseDuration;
      lastPowerPulseStart                 = xTaskGetTickCount();
      lastPowerPulseEnd                   = lastPowerPulseStart + powerPulseDuration;
    }

    // If current PID is less than the pulse level, check if we want to constrain to the pulse as the floor
    if (x10WattsOut < systemSettings.KeepAwakePulse && xTaskGetTickCount() < lastPowerPulseEnd) {
      x10WattsOut = systemSettings.KeepAwakePulse;
    }
  }

  // Secondary safety check to forcefully disable header when within ADC noise of top of ADC
  if (getTipRawTemp(0) > (0x7FFF - 32)) {
    x10WattsOut = 0;
  }
  if (heaterThermalRunaway) {
    x10WattsOut = 0;
  }
#ifdef SLEW_LIMIT
  if (x10WattsOut - x10WattsOutLast > SLEW_LIMIT) {
    x10WattsOut = x10WattsOutLast + SLEW_LIMIT;
  }
  if (x10WattsOut < 0) {
    x10WattsOut = 0;
  }
  x10WattsOutLast = x10WattsOut;
#endif
  setTipX10Watts(x10WattsOut);
  resetWatchdog();
}








#else
#include "Threads.hpp"

static TickType_t powerPulseRate = 10000;
static TickType_t powerPulseDuration = 250;
TaskHandle_t pidTaskNotification = NULL;	//PID任务通知的任务句柄

uint32_t currentTempTargetDegC = 0; // 当前目标温度 Current temperature target in C

template<class T, uint8_t SIZE>
struct history {
	static const uint8_t size = SIZE;	//static const 类成员，为所有类的实例共享的数据，必须在类内对其初始化
	T buf[size];
	int32_t sum;	//T类型值的总和
	uint8_t loc;	//元素编号（location）

	void update(T const val) {
		// step backwards so i+1 is the previous value.
		//向后退一步，因此i + 1是前一个值。

		sum -= buf[loc];
		sum += val;
		buf[loc] = val;
		loc = (loc + 1) % size;	//loc0+8次1才会使loc=1？
	}

	//下标运算符 [] 重载
	T operator[](uint8_t i) const {
		// 0 = newest, size-1 = oldest.
		i = (i + loc) % size;
		return buf[i];
	}

	T average() const {
		return sum / size;
	}
};

/* PIDTask function */
void doPIDTask() {
	/*
	 * We take the current tip temperature & evaluate the next step for the tip
	 * control PWM.
	 * 我们获取当前烙铁头温度，并计算烙铁头下一次PWM的控制量。
	 */
	setTipX10Watts(0);    // force tip off
						  // disable the output driver if the output is set to be off
						  //如果输出设置为关闭，则禁用输出驱动器
						  //传入0mW,采样1次电池电压
	TickType_t lastPowerPulseStart = 0;
	TickType_t lastPowerPulseEnd = 0;

	history<int32_t, PID_TIM_HZ> tempError = { { 0 }, 0, 0 };//用于储存tError即温度偏差值
	currentTempTargetDegC = 0; // Force start with no output (off). If in sleep / soldering this will
							   // be over-ridden rapidly
							   //开始时强制无输出（关闭）。在睡眠/焊接切换过程中该值将迅速被修改

	pidTaskNotification = xTaskGetCurrentTaskHandle();	//=POWTaskHandle? 获取当前正在运行的任务的任务句柄
	uint32_t PIDTempTarget = 0;
	for (;;) {
#if 0
		if(pidTaskNotification == NULL)
		usb_printf("pidTaskNotification == NULL\r\n");
		else
			usb_printf("pidTaskNotification == %d\r\n", pidTaskNotification);
		osDelay(100);
#endif
		//currentTempTargetDegC = systemSettings.SolderingTemp;	//320
	/**
	 * 获取任务通知，可以设置在退出此函数的时候将任务通知值清零 或者减一。
	 * 当任务通知用作二值信号量或者计数信号量的时候使 用此函数来获取信号量
	 */
		if (ulTaskNotifyTake(pdTRUE, 2000)) {
			// 在ADC进行采样之前阻塞该线程的调用
			// This is a call to block this thread until the ADC does its samples
			int32_t x10WattsOut = 0;
			// Do the reading here to keep the temp calculations churning along
			uint32_t currentTipTempInC = TipThermoModel::getTipInC(true);

			// PID目标温度, 有一定的超调
			PIDTempTarget = currentTempTargetDegC;

			if (PIDTempTarget) {
				// Cap the max set point to 450C
				// 将最大设置点设置为450C
				if (PIDTempTarget > (450)) {
					// Maximum allowed output
					PIDTempTarget = (450);
				}
				// 安全检查，目标不超过当前尖端
				// Safety check that not aiming higher than current tip can measure
				if (PIDTempTarget > TipThermoModel::getTipMaxInC()) {
					PIDTempTarget = TipThermoModel::getTipMaxInC();
				}

				//将当前的笔尖转换为摄氏度
				// Convert the current tip to degree's C

				//当我们接近目标时，温度噪声会导致系统
				//变得不稳定。 使用滚动平均值对其进行衰减。
				//我们会超出大约1摄氏度。(最小维持控制量是1摄氏度的功率)
				//这有助于稳定显示。
				// As we get close to our target, temp noise causes the system
				//  to be unstable. Use a rolling average to dampen it.
				// We overshoot by roughly 1 degree C.
				//  This helps stabilize the display.

				//最小维持控制量 改为在HAL_TIM_PeriodElapsedCallback回调函数中将TIM3_CCR1置为1
				//int32_t tError = PIDTempTarget - currentTipTempInC + 1;//这里加不起作用,升温时温度会乱串
				int32_t tError = PIDTempTarget - currentTipTempInC;

				tError = tError > INT16_MAX ? INT16_MAX : tError;
				tError = tError < INT16_MIN ? INT16_MIN : tError;
				tempError.update(tError);

// Now for the PID!

				// P term - total power needed to hit target temp next cycle.
				// thermal mass = 1690 milliJ/*C for my tip.
				//  = Watts*Seconds to raise Temp from room temp to +100*C, divided by 100*C.
				// we divide milliWattsNeeded by 20 to let the I term dominate near the set point.
				//  This is necessary because of the temp noise and thermal lag in the system.
				// Once we have feed-forward temp estimation we should be able to better tune this.
		//比例项
				// P term - 下一个周期达到目标温度所需的总功率。
				// 热质量 = 1690 milliJ/*C 对于我的小烙铁。
				// = 将温度从室温升高到 +100*C 的瓦特*秒，除以 100*C。
				// 我们将milliWattsNeeded除以20，让I项在设定点附近占主导地位。
				// 这是必要的，因为系统中存在温度噪声和热滞后。
				// 一旦我们有了前馈温度估计，我们应该能够更好地调整它。

				int32_t x10WattsNeeded = tempToX10Watts(tError);
				// note that milliWattsNeeded is sometimes negative, this counters overshoot
				//  from I term's inertia.
				// 请注意，milliWattsNeeded 有时为负数，这会抵消 I 项惯性的过冲。
				x10WattsOut += x10WattsNeeded;
		//积分项
				// I term - energy needed to compensate for heat loss.
				// We track energy put into the system over some window.
				// Assuming the temp is stable, energy in = energy transfered.
				//  (If it isn't, P will dominate).
				// I term - 补偿热损失所需的能量。
				// 我们跟踪通过某个窗口输入系统的能量。
				// 假设温度稳定，能量输入 = 能量转移。
				//（如果不是，P 将占主导地位）。
				x10WattsOut += x10WattHistory.average();
		//微分项
				// D term - use sudden temp change to counter fast cooling/heating.
				//  In practice, this provides an early boost if temp is dropping
				//  and counters extra power if the iron is no longer losing temp.
				// basically: temp - lastTemp
				//  Unfortunately, our temp signal is too noisy to really help.
				// D 项 - 使用突然的温度变化来应对快速冷却/加热。
				// 实际上，如果温度下降，这会提供早期提升
				// 如果熨斗不再失去温度，则抵消额外的功率。
				// 基本上：temp - lastTemp
				// 不幸的是，我们的温度信号太嘈杂，无法真正提供帮助。
			}
			// If the user turns on the option of using an occasional pulse to keep the power bank on
			if (systemSettings.KeepAwakePulse) {

				if (xTaskGetTickCount() - lastPowerPulseStart
						> powerPulseRate) {
					lastPowerPulseStart = xTaskGetTickCount();
					lastPowerPulseEnd = lastPowerPulseStart
							+ powerPulseDuration;
				}

				// If current PID is less than the pulse level, check if we want to constrain to the pulse as the floor
				if (x10WattsOut < systemSettings.KeepAwakePulse
						&& xTaskGetTickCount() < lastPowerPulseEnd) {
					x10WattsOut = systemSettings.KeepAwakePulse;
				}
			}

			// Secondary safety check to forcefully disable header when within ADC noise of top of ADC
			// 输入电压小于8V,或手柄温度大于70,或菜单页面停止加热时，也关闭PID输出, 7FFF = 32767
			if ( //!(tipState == TIP_HEATING || tipState == TIP_SLEEPING_TIME_OUT) ||
					getTipRawTemp(0) > (0x7FFF - 150) ||
					((getInputVoltageX10()/10) < 8) ||
					(getHandleTemperature()/10) > 70) {
				x10WattsOut = 0;
			}

			//归并功率到有效范围
			if (systemSettings.powerLimit
					&& x10WattsOut > (systemSettings.powerLimit * 10)) {	//若PID计算的功率比限制值大
				setTipX10Watts(systemSettings.powerLimit * 10);				//输出限制值
			} else {
				//setTipX10Watts(x10WattsOut);
				setTipX10Watts(100);	//否则输出计算值
			}
		} else {
			// ADC中断超时
			// ADC interrupt timeout
			setTipPWM(0);
		}
		resetWatchdog();
	}
}

//int16_t datax[MOVFilter];
#endif
