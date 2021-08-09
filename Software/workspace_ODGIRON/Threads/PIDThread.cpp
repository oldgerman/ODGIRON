/*
 * MOVThread.cpp
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 *      Modify: OldGerman
 */

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
				setTipX10Watts(x10WattsOut);								//否则输出计算值
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
