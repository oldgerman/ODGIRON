 /*
 * LIS2DH12_defines.hpp
 *
 *  Created on: 27Feb.,2018
 *      Author: Ralim
 */

#ifndef LIS3DH_DEFINES_HPP_
#define LIS3DH_DEFINES_HPP_

//ADDR
#define LIS3DH_I2C_ADDRESS (25 << 1)	//25D=11001b=0x19（7bit i2c地址）	//0001101xb  x：write=0 read=1，他这里只写，就将地址向右恒定偏移1，即00110010

//所有的SUB字段都是自增模式+要读取的寄存器地址
//CTRL
#define LIS_CTRL_REG1     0x20 | 0x80	// 0x80=10000000b 为了连续读取多个字节，SUB字段的MSb必须为“ 1”，会是SUB（寄存器地址）自动增加以允许多次数据读/写。而SUB（6-0）代表要读取的第一个寄存器的地址。
#define LIS_CTRL_REG2     0x21 | 0x80
#define LIS_CTRL_REG3     0x22 | 0x80
#define LIS_CTRL_REG4     0x23 | 0x80
#define LIS_CTRL_REG5     0x24 | 0x80
#define LIS_CTRL_REG6     0x25 | 0x80

//Axis
#define LIS3DH_OUT_X_L    0x28 | 0x80
#define LIS3DH_OUT_X_H    0x29 | 0x80
#define LIS3DH_OUT_Y_L    0x2A | 0x80
#define LIS3DH_OUT_Y_H    0x2B | 0x80
#define LIS3DH_OUT_Z_L    0x2C | 0x80
#define LIS3DH_OUT_Z_H    0x2D | 0x80

//INT1
#define LIS_INT1_CFG      0xB0 | 0x80
#define LIS_INT2_CFG      0xB4 | 0x80
#define LIS_INT1_DURATION 0x33 | 0x80
#define LIS_INT1_THS      0x32 | 0x80
#define LIS_INT1_SRC      0x31 | 0x80

//INT2
#define LIS_INT2_DURATION 0x37 | 0x80
#define LIS_INT2_THS      0x36 | 0x80
#define LIS_INT2_SRC      0x35 | 0x80
#endif /* LIS2DH12_DEFINES_HPP_ */
