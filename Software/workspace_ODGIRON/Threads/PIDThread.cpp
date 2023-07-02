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
int32_t getX10WattageLimits();

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
		// 由ADC注入通道转换完成后的HAL_ADCEx_InjectedConvCpltCallback()函数向PIDTask发送的任务通知
		if (ulTaskNotifyTake(pdTRUE, 1000)) {
			// Do the reading here to keep the temp calculations churning along
			uint32_t currentTipTempInC = TipThermoModel::getTipInC(true);//瞬时采集温度
			PIDTempTarget = currentTempTargetDegC + systemSettings.balanceTempOffsetPositive - systemSettings.balanceTempOffsetnegative;	//目标温度
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
				//tError = 目标温度 - 当前温度
				//那么tError可正可负，但，若是未插入烙铁，则当前温度为ADC最大值，tError也为负值
				//如何避免detectThermalRunaway()将此情况当作失控而触发保护?
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
#if 1	//修改为可调的
	if (systemSettings.ThermalRunawayProtectionEnable) {
		static uint16_t tipTempCRunawayTemp = 0;
		static TickType_t runawaylastChangeTime = 0;
		static bool firstIn = true; //（一个保护触发到退出的循环的）首次进入的保护标记，
									//用作本次触发保护使heaterThermalRunaway = true 时 但按下AB键使heaterThermalRunaway = false时，也能重置计时变量
		// Check for thermal runaway, where it has been x seconds with negligible (y) temp rise
		// While trying to actively heat

		// If we are more than 20C below the setpoint
		// 这个tError只能检查低于温度的保护
		if ((tError > systemSettings.ThermalRunawayTempC)) {

			// If we have heated up by more than 20C since last sample point, snapshot time and tip temp
			// 如果自上一个采样点以来我们已经升温超过 20C，快照时间和尖端温度
			// 每次进来都计算一次delta
			int16_t delta = (int16_t) currentTipTempInC
					- (int16_t) tipTempCRunawayTemp;
			if (delta > systemSettings.ThermalRunawayTempC || (!firstIn && !heaterThermalRunaway)) {
				// We have heated up more than the threshold, reset the timer
				tipTempCRunawayTemp = currentTipTempInC;
				runawaylastChangeTime = xTaskGetTickCount();
				firstIn = true;
				/*++*/
				//heaterThermalRunaway = false;//改为放在doGUITask的switch(buttons)
				/*++*/
			}
			else	//否则，当超时后判定为 heaterThermalRunaway
			{
				if ((xTaskGetTickCount() - runawaylastChangeTime)
						> (systemSettings.ThermalRunawayTimeSec * TICKS_SECOND)) {
					// It has taken too long to rise
					heaterThermalRunaway = true;
					firstIn = false;
				}
			}
		} else {
			tipTempCRunawayTemp = currentTipTempInC;
			runawaylastChangeTime = xTaskGetTickCount();
		}
	} else
		return;
#else	//Ralim's code 只能检测温度低于阈值的情况
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
#endif
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
