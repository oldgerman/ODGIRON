/*
 * LIS3DH.cpp
 *
 *  Created on: 27Feb.,2018
 *      Author: Ralim
 */

#include <array>

#include "LIS3DH.hpp"
#include "cmsis_os.h"

/*
 * ironOS的LIS3DH工作在12bit 1mg/digit，即[-2048, +2048]mg，即量程±2g
 * 注意：不是毫克和克，是加速度g
 * 实测：自由落体不会触发Movement detected
 * 疑问：某一个方向的轴加速度受地球重力影响，本身就为1g啊，它怎么消除这个恒定偏差的？
 * ironOS原版配置的 CTRL_REG1 (20h)的ODR[3:0]，为0011，即ODR=25Hz
 * 那么High-resolution的BW就是2.77Hz，即刷新周期360ms，而MOVTask每100ms读一次，那比它刷新快啊
 * 我改为100Hz，那么12bit输出速率就是100/9 = 11.11次/s，与MOVtask读的速率差不多
 */
static const FRToSI2C::I2C_REG i2c_registers[] = {
													  //CTRL_REGx
					/*原IronOS错了，运动检测很迟钝*///{LIS_CTRL_REG1, 0x17, 0},       //default：00000111 // 25Hz               			// 0x17 = 0b00010111，查表是1Hz，注释错了
 /*我查表改正确，运动检测很灵敏，但比MOVTask刷新慢*///{LIS_CTRL_REG1, 0b00110111, 0},   //default：00000111 // 数据速率:  25Hz               // 0b00110111，查表Table 31. Data rate configuration 是25Hz
 /*我查表改100Hz，运动检测很灵敏，比MOVTask刷新快*/   {LIS_CTRL_REG1, 0b01010111, 0},   //default：00000111 // 数据速率: 100Hz               // 0b01010111，查表Table 31. Data rate configuration 是100Hz

		/*不要，还是开高通滤波器，否则输出3轴数据为偏差值*///
 	 	 	 	 	 	 	 	 	 	 	 {LIS_CTRL_REG2, 0b00001000, 0}, //default：00000000 // Highpass filter off          // 		 FDS[3:3] = 1：来自内部滤波器的数据发送到输出寄存器和FIFO
													  {LIS_CTRL_REG3, 0b01100000, 0}, //default：00000000 // Setup interrupt pins         // 	  I1_IA1[1:1] = 1：IA1 interrupt on INT1
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	///   I1_IA2[2:2] = 1：IA2 interrupt on INT2
													  {LIS_CTRL_REG4, 0b00001000, 0}, //default：00000000 // Block update mode off, HR on // 	      HR[4:4] = 1：高分辨率输出模式 FS[1:0] = 00  即±2g
													  {LIS_CTRL_REG5, 0b00000010, 0}, //default：00000000 // 							  //  	LIR_INT2[7:7] = 1：在INT2_SRC（35h）寄存器上锁存中断请求，并通过读取INT2_SRC（35h）本身清除INT2_SRC（35h）寄存器。
													  //{LIS_CTRL_REG6, 0b01100010, 0}, //default：00000000 //						      //      I2_IA1[1:1] = 1：Enable interrupt 1 function on INT2 pin
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	    ///   I2_IA2[2:2] = 1：Enable interrupt 2 function on INT2 pin
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	    ///   I2_ACT[6:6] = 1：Enable activity interrupt on INT2 pin

													  // Basically setup the unit to run, and enable 4D orientation detection
													  // 运行单元的基本设置，并启用4D方向检测 <---根据Table64，它开了6轴向运动检测中断，不开方位检测中断
													  {LIS_INT2_CFG, 0b01111110, 0}, //default：00000000 // setup for movement detection  //	      6D[1:1] = 1：6-direction detection function enabled
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	///     ZHIE[2:2] = 1：Enable interrupt generation on Z high event：1: enable interrupt request on measured accel. value higher than 【preset threshold】（预设阈值）)
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	///		ZLIE[3:3] = 1：Enable interrupt generation on Z low  event：1: enable interrupt request on measured accel. value lower  than preset threshold)
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	    ///		YHIE[4:4] = 1：~
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	///		YLIE[5:5] = 1：~
													  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	///		XHIE[6:6] = 1：~	 //为啥没启用XLIE[7:7] ???
													  {LIS_INT2_THS, 45, 0},       //default：00000000 // 6轴向运动检测共用阈值的绝对值  //由±2g时 1LSB = 16mg，算得触发阈值 = 50DEC*16msg/LSB = 800mg 而12bit ±2g为1LSB=1mg，则读数为±800触发临界点, 实测x轴读数±800左右触发
						/*开了没开一样，没卵用？*/	  {LIS_INT2_DURATION, 64, 0},    //default：00000000 //  							  // 64 = 0b01000000	// 超出阈值持续多少时间才触发INT2的中断（相当于按键延时消抖）

													  //{LIS_INT1_CFG, 0b01111110, 0}, //default：00000000 //								  //
													  //{LIS_INT1_THS, 0x28, 0},       //default：00000000 //								  //0x28 = 0b00101000
													  //{LIS_INT1_DURATION, 64, 0}	 //default：00000000 //							      //  64 = 0b01000000
};

//FRToSI2C::I2C_REG i2c_registers[]配合FRToSI2C::writeRegistersBulk()作初始化i2c设备的寄存器配置用
bool LIS3DH::initalize() { return FRToSI2C2.writeRegistersBulk(LIS3DH_I2C_ADDRESS, i2c_registers, sizeof(i2c_registers) / sizeof(i2c_registers[0])); }


/**
 * @brief  读取三轴数据
 * @param  传x、y、z数据的引用
 * @retval none
 */
void LIS3DH::getAxisData(AxisData  *axisData) {
  /*
   * XYZ-axis acceleration data. The value is expressed as two’s complement left-justified. Please refer to Section 3.2.1: High-resolution, normal mode, low-power mode.
   * XYZ轴加速度数据。一个轴的12bit值由两个左对齐的8bit数据组合表示
   * 0x28、0x29、0x2A、0x2B、0x2C、0x2B
   *
   * 8.14 OUT_X_L (28h), OUT_X_H (29h)
   * 8.15 OUT_Y_L (2Ah), OUT_Y_H (2Bh)
   * 8.16 OUT_Z_L (2Ch), OUT_Z_H (2Dh)
   */
  std::array<int16_t, 3> sensorData;	//三组16bit用于存储三个轴12bit数据
  FRToSI2C2.Mem_Read(
		  	  	  	  LIS3DH_I2C_ADDRESS,
					  LIS3DH_OUT_X_L/*0xA8*/,
					  reinterpret_cast<uint8_t *>(sensorData.begin()), // 将定位到容器首元素iterator强制转换为unit8_t*,什么炫技操作？
					  sensorData.size() * sizeof(int16_t)
					);

//裁剪16bit内多余的4bit未定义的数据，不然会读到10000多的非法值，正常范围是[-2048，+2048]
#if 0
  sensorData[0] <<= 4;
  sensorData[1] <<= 4;
  sensorData[2] <<= 4;

  sensorData[0] >>= 4;
  sensorData[1] >>= 4;
  sensorData[2] >>= 4;
#endif

  //依次赋值
  axisData->x = sensorData[0]>>4;
  axisData->y = sensorData[1]>>4;
  axisData->z = sensorData[2]>>4;

	//uint8_t myBuffer[6];
	//FRToSI2C::Mem_Read(LIS3DH_I2C_ADDRESS, LIS3DH_OUT_X_L, myBuffer, 6);  //Does memory transfer
 #if 0
	int16_t *ptr = (int16_t *)axisData;
	for(int i = 0; i < 3; i++ )
		*(ptr+i) = (int16_t)myBuffer[2*i] | int16_t(myBuffer[2*i+1] << 8);

#else
#if 0
	axisData->x = ((int16_t)myBuffer[0] | int16_t(myBuffer[1] << 8))>> 4;
	axisData->y = ((int16_t)myBuffer[2] | int16_t(myBuffer[3] << 8))>> 4;
	axisData->z = ((int16_t)myBuffer[4] | int16_t(myBuffer[5] << 8))>> 4;
#elif 0
	axisData->x = ((int16_t)myBuffer[0] | int16_t(myBuffer[1] << 8));
	axisData->y = ((int16_t)myBuffer[2] | int16_t(myBuffer[3] << 8));
	axisData->z = ((int16_t)myBuffer[4] | int16_t(myBuffer[5] << 8));
#endif
#endif
#if 0
	//int16_t result_lsb;
	//result_lsb = ((int16_t)high_byte<<8)+(uint16_t)low_byte;
	//result_lsb >>= 4; // in case of high resolution mode = 12bit output
	//result_mg = result_lsb * sensitivity; // LSB to mg, see sensitivity in datasheet
	int16_t *ptr = (int16_t *)axisData;
	for(int i = 0; i < 3; i++ )
		*(ptr+i) = (((int16_t)myBuffer[2*i+1] << 8) + (int16_t)myBuffer[2*i]) >> 4;

#endif
}

/**
 * @brief  检测地址是否有ACK (i2c应答)
 * @param
 * @retval Orientation枚举类
 */
bool LIS3DH::detect() { return FRToSI2C2.probe(LIS3DH_I2C_ADDRESS); }


/**
 * @brief  获得方向（ODGIRON版本）
 * @param
 * @retval Orientation枚举类
 * LIS_INT2_SRC用于旋转方向检测，值有以下三种:
 * XL event: 000001 = 1 = rh, 即right
 * XH event: 000010 = 2 = lh, 即left
 * ZL event: 010000 = 16 = flat
 * 注意使能了CTRL_REG5 的LIR_INT2位：在INT2_SRC 寄存器上锁存中断请求，只有读取了 INT2_SRC 才会继续刷新其本身的值
 * INT2_SRC:
 * 0 IA ZH ZL YH YL XH XL
 * 0  1  ?  ?  ?  ?  ?  ?
 */
Orientation LIS3DH::getOrientation() {
#ifdef LIS_ORI_FLIP
    uint8_t val = (FRToSI2C2.I2C_RegisterRead(LIS3DH_I2C_ADDRESS, LIS_INT2_SRC));
    if (val == 16)//若触发ZL事件，视为平坦模式
      val = 3;
    else if (val == 1)//若触发XL事件，视为左手模式
      val = 1;
    else if (val == 2)//若触发XH事件，视为右手模式
      val = 0;
    else	//若未触发任何阈值事件，也视为平坦模式
      val = 3;
    return static_cast<Orientation>(val);
#else
    return static_cast<Orientation>((FRToSI2C2.I2C_RegisterRead(LIS2DH_I2C_ADDRESS, LIS_INT2_SRC) >> 2) - 1);
    //					^将LIS2DH12内置的4D/6D计算单元的返回值转换为对应的枚举类型的枚举成员
#endif
}


AxisFloat LIS3DH::getAxisFloat(AxisData * axisData)
{
	AxisFloat axisFloat;
#if 0
	//高精度模式，±2g 12bit，相当于上面的case 2:
	int16_t *ptr = (int16_t *)axisData;
	float *ptrf = (float *)&axisFloat;
	for(int i = 0; i < 3; i++ )
		*(ptrf + i) = (float)*(ptr + i) / 15987;
#else
	//axisFloat.x = (float)(axisData->x) / 15987;
	//axisFloat.y = (float)(axisData->y) / 15987;
	//axisFloat.z = (float)(axisData->z) / 15987;
	axisFloat.x = (float)(axisData->x) / 16;
	axisFloat.y = (float)(axisData->y) / 16;
	axisFloat.z = (float)(axisData->z) / 16;


#endif
	return axisFloat;
}
