/*
 * MOVThread.cpp
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 *      Modify: OldGerman
 */

#include <Buttons.hpp>
#include <Threads.hpp>
#include "LIS3DH.hpp"
#include "power.hpp"
#include "stdlib.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif

#include "I2C_Wrapper.h"
uint8_t DetectedAccelerometerVersion = 0;		//检测加速度计型号的标记

uint8_t accelInit = 0;		// 首次标记
TickType_t lastMovementTime = 0;//首次进入shouldShuntDown()函数置为1才会累计时间
bool moveDetected = false;	//运动检测标记

#define MOVFilter 8
int32_t axisError = 0;
AxisData axisData;
AxisAvg axAvg;

//int16_t tx, ty, tz;	//从加速读机读取的12bit数据是有符号整型嗷，读到值的范围为 [-2048, +2048]


/**
 * @brief  检测加速度计型号，并做寄存器初始化配置
 * @param  void
 * @retval void
 */
void detectAccelerometerVersion() {
	DetectedAccelerometerVersion = ACCELEROMETERS_SCANNING;

#ifdef ACCEL_LIS
	if (LIS3DH::detect()) {
		// Setup the ST Accelerometer
		if (LIS3DH::initalize()) {
			DetectedAccelerometerVersion = 2;
		}
	} else
#endif
	{
		// disable imu sensitivity，没检测到加速度计，会禁用休眠模式
		systemSettings.Sensitivity = 0;
		DetectedAccelerometerVersion = NO_DETECTED_ACCELEROMETER;
	}
}

/**
 * @brief  读取加速度计
 * @param  x，y，z，枚举方向
 * @retval void
 */
inline void readAccelerometer(AxisData *axisData, Orientation &rotation) {
#ifdef ACCEL_LIS
	if (DetectedAccelerometerVersion == 2) {
		LIS3DH::getAxisData(axisData);	//先读一次3轴向数据
		rotation = LIS3DH::getOrientation();	//再读一次姿态
	} else
#endif
	{
		// do nothing :(
	}
}

void doMOVTask() {


	// 这是因为BMA(BMA250E加速度计)不会立即启动，并且如果引导后探测得太快，则会楔入I2C总线
	// This is here as the BMA doesnt start up instantly and can wedge the I2C bus if probed too fast after boot
	osDelay(TICKS_100MS / 5);

	// 检测加速度计型号，并做寄存器初始化配置
	detectAccelerometerVersion();

	osDelay(TICKS_100MS / 2); // wait ~50ms for setup of accel to finalise

	// 清零没运动的计时标记值（用于判断超时休眠）
	lastMovementTime = 0;

	// 屏蔽2秒钟（如果我们处于自动启动状态），以便用户插入（烙铁头？）
	// 然后站起来不会立即唤醒
	// Mask 2 seconds if we are in autostart so that if user is plugging in and
	// then putting in stand it doesnt wake instantly
	if (systemSettings.AutoStartMode)	//
		osDelay(2 * TICKS_SECOND);

	int16_t datax[MOVFilter] = { 0 };
	int16_t datay[MOVFilter] = { 0 };
	int16_t dataz[MOVFilter] = { 0 };
	//本次task
	uint8_t currentPointer = 0;
	axisData.x = 0;
	axisData.y = 0;
	axisData.z = 0;
	//加速度计灵敏度，在GUI.cpp中可以用按键设置加速度计灵敏度并oled打印出来，
	//注意不要和加速度计本身的12bit分辨率输出值搞混了


	//默认姿态：平坦
	Orientation rotation = ORIENTATION_FLAT;

	/**
	 * @brief  判断是否超出阈值，若超出更新休眠计时标记
	 * @param  None
	 * @retval None
	 * @task   每100ms
	 * @疑问   1) LIS2DH12是12bit，LIS3DH是16bit，那阈值上会不会差4bit?
	 * 		   2) 是否读FIFO: LIS3DH: 16-bit, 32-level;  LIS2DH12: 10-bit, 32-level
	 */
	int32_t threshold;
	uint8_t valVolume;	//蜂鸣器音量映射值
	//lastMovementTime = xTaskGetTickCount();
	for(;;) {
		//阈值（threshold）配置
		//int32_t threshold = 1500 + (9 * 200);
		//							  ^
		//int32_t threshold = 1500;	//轻轻摇摇2cm就能检测到移动
		/*阈值减去用户设置的灵敏度，用户设置值越大，阈值越小，灵敏度越高
		 * ironOS默认是2500（你单轴最大才2048?需要三轴变化总和大于2500，单轴无法达到2048，两轴最大根号2*2048=2896，三轴最大3547）
		 * 现在是若执行下一句是700，不执行是1500
		 */
		threshold = map(systemSettings.Sensitivity, 0, 100, 0, 2048);
		//threshold -= systemSettings.sensitivity * 200; // 200 is the step size
			//模拟正常焊接拿着到处动不会触发，下意识摇动能立即检测到移动

		readAccelerometer(&axisData, rotation);

		if (systemSettings.OrientationMode == 2) {
			if (rotation != ORIENTATION_FLAT) { //非平坦状态才会旋转屏幕
				//rotation是否等于左手的值0？
				//是则rotation=0, 即右手模式, 传入OLED::setRotation(1)
				//否则rotation=1, 即左手模式, 传入OLED::setRotation(0)
				//OLED::setRotation(rotation == ORIENTATION_LEFT_HAND); // link the data through
				u8g2.setDisplayRotation((rotation == ORIENTATION_LEFT_HAND)? U8G2_R2: U8G2_R0);
			}
		}
		//usb_printf("rotation = %d\r\n", rotation);
		datax[currentPointer] = (int32_t) axisData.x;
		datay[currentPointer] = (int32_t) axisData.y;
		dataz[currentPointer] = (int32_t) axisData.z;

		// 首次进入循环先将读到的一次三轴数据复制8份（MOVFilter = 8），塞满datax、datay、dataz的8个元素
		// 离开将首次标记accelInit置1，下次task不会再执行本if内的语句
		if (!accelInit) {
			for (uint8_t i = currentPointer + 1; i < MOVFilter; i++) {
				datax[i] = (int32_t) axisData.x;
				datay[i] = (int32_t) axisData.y;
				dataz[i] = (int32_t) axisData.z;
			}
			accelInit = 1;
		}

		currentPointer = (currentPointer + 1) % MOVFilter; //整除则变为0，从0重新计数到7
		axAvg.avgx = axAvg.avgy = axAvg.avgz = 0;
		// calculate averages

		for (uint8_t i = 0; i < MOVFilter; i++) {
			axAvg.avgx += datax[i];
			axAvg.avgy += datay[i];
			axAvg.avgz += dataz[i];
		}

		axAvg.avgx /= MOVFilter;
		axAvg.avgy /= MOVFilter;
		axAvg.avgz /= MOVFilter;

		// 求三轴变化量绝对值的总和 //Sum the deltas	//abs()求传入数据的绝对值
		axisError = (abs(axAvg.avgx - axisData.x) + abs(axAvg.avgy - axisData.y)
				+ abs(axAvg.avgz - axisData.z));
		// So now we have averages, we want to look if these are different by more
		// than the threshold

		// If movement has occurred then we update the tick timer
		// 如果发生了移动，那么我们更新滴答计时器
		if (axisError > threshold) {
			lastMovementTime = xTaskGetTickCount();	//更新动作时间
			usb_printf("Movement detected!\r\n");		//若触发移动则打印
			moveDetected = true;
		}
#ifdef   DEBUG
		usb_printf("Debug is define!\r\n");
#endif
		usb_printf("Hello World!\r\n");
		//DetectedAccelerometerVersion = 2 识别LIS3DH OK
		//usb_printf("DetectedAccelerometerVersion = %d\r\n", DetectedAccelerometerVersion);
		//usb_printf("tx = %d ty= %d tz = %d rotation = %d ptr= %d\r\n", axisData.x, axisData.y, axisData.z, rotation, currentPointer);
		//AxisFloat axisFloat = LIS3DH::getAxisFloat(&axisData);
		//usb_printf("X: %.2f  Y: %.2f  Z: %.2f (g)\r\n", axisFloat.x, axisFloat.y, axisFloat.z);
		//usb_printf("X: %d  Y: %d  Z: %d (g)\r\n", (int16_t)axisFloat.x, (int16_t)axisFloat.y, (int16_t)axisFloat.z);

		//蜂鸣器
		if(buttonChanged)
		{
			buttonChanged = false;
			buttonIsBeep = false;
			valVolume = map(systemSettings.BuzzerVolume, 0, 100, 0, 128);
			if(valVolume)
			{
				htim3.Instance->CCR2 = valVolume;
				HAL_TIM_PWM_Start(&htim3, BUZZER_CHANNEL);	//buzzer
			}
		}
		else
		{
			//将CCR1置为0或255时，蜂鸣器仍有底噪异响，必须关闭PWM通道才安静
			//htim3.Instance->CCR1 = 0;
			HAL_TIM_PWM_Stop(&htim3, BUZZER_CHANNEL);	//buzzer
			buttonIsBeep = true;	//下次进入时才能置为true，每次响必定经过两倍的周期时间，即200ms（100ms太短不好听）
		}

		osDelay(TICKS_100MS); // Slow down update rate	//每100ms调用一次
		//osDelay(500);
	}
}
