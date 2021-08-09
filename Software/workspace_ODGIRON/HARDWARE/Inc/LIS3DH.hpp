/*
 * LIS3DH.hpp
 *
 *  Created on: 27Feb.,2018
 *      Author: Ralim
 */

#ifndef LIS3DH_HPP_
#define LIS3DH_HPP_
#include "BSP.h"
#include "main.h"
#include "I2C_Wrapper.h"
#include "LIS3DH_defines.hpp"


/*
 * 疑问：
 * 		1）这个中断, 它开了LIS2GH12两个GPIO的中断引脚映射
 * 			到底是挂为task去周期性地读LIS3DH的运动检测中断标志的寄存器，
 * 			还是开了stm32F103的GPIO外部中断，在回调函数里处理？
 * 			还是周期性地读取GPIO电平？
 * 			答：没有找到GPIO EXIT注册NVIC的地方，目前可以肯定INT2，而INT1未知，它在寄存器组中做了以下配置：
 * 			 {LIS_CTRL_REG5, 0b00000010, 0}, //  LIR_INT2[7:7] = 1：在INT2_SRC（35h）寄存器上锁存中断请求，并通过读取INT2_SRC（35h）本身清除INT2_SRC（35h）寄存器。
 * 			 因为INT2的中断会锁存，只有读取了才会清除，所以可以由task周期性地读取，不需要注册GPIO EXTI
 * 		2）既然支持运动检测唤醒，那class LIS3DH咋没有相关API呢，难道直接用FRToSI2C::I2C_RegisterRead()？
 * 			答
 *
 */
typedef struct axisData
{
	int16_t x;
	int16_t y;
	int16_t z;
}AxisData;

typedef struct axisFloat
{
	float x;
	float y;
	float z;
}AxisFloat;


#ifdef __cplusplus

extern FRToSI2C FRToSI2C2;

class LIS3DH {
public:
  static bool detect();
  static bool initalize();
  static void getAxisData(AxisData *);	//为啥还需要获取轴向加速度数据？不是开了硬件阈值中断嘛
  static Orientation getOrientation();
  static void calcAccel(AxisData *);
  static AxisFloat getAxisFloat(AxisData *);

private:
};

extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif /* LIS3DH_HPP_ */
