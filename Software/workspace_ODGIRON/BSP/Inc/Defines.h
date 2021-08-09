/*
 * Defines.h
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 */

#ifndef BSP_DEFINES_H_
#define BSP_DEFINES_H_

// 加速度计方向的枚举类，用于oled旋转
enum Orientation {
	ORIENTATION_LEFT_HAND = 1,		//左手
	ORIENTATION_RIGHT_HAND = 0,		//右手
	ORIENTATION_FLAT = 3			//平坦的
};

// It is assumed that all hardware implements an 8Hz update period at this time
// 假设此时所有硬件都实现了8Hz更新周期（稍后会配置TIM2溢出频率为8Hz）
#define PID_TIM_HZ   (8)					//PID计算周期，也是TIM2的周期？
#define TICKS_SECOND configTICK_RATE_HZ		//1秒;	configTICK_RATE_HZ 为 FreeRTOS 的系统时钟节拍频率，单位为HZ
#define TICKS_MIN    (60 * TICKS_SECOND)	//1分钟
#define TICKS_100MS  (TICKS_SECOND / 10)	//100ms
#endif /* BSP_DEFINES_H_ */
