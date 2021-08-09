/*
 * oled_init.h
 *
 *  Created on: Jan 17, 2021
 *      Author: 29209
 */


#ifndef __OLED_INIT_H
#define __OLED_INIT_H

#include "u8g2.h"
#include "U8g2lib.h"
#include "main.h"

//#define OLED_I2C_NONE_DMA
#define OLED_I2C_WITH_DMA


//#include "i2c.h"
#define U8X8_PIN_NONE 255

#ifdef __cplusplus
extern "C" {
#endif
//#define DEVICE_ADDRESS 0x3D//0X78
#define DEVICE_ADDRESS 0x3C
#define TX_TIMEOUT 100
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

#ifdef OLED_I2C_NONE_DMA
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#endif

#ifdef OLED_I2C_WITH_DMA
uint8_t u8x8_byte_stm32hal_hw_i2c_dma(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#endif

extern uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t psoc_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);


extern void u8g2Init(u8g2_t *u8g2);
void u8g2_begin();

#ifdef __cplusplus

class U8G2_SSD1306_128X32_UNIVISION_HW_I2C : public U8G2 {
  public: U8G2_SSD1306_128X32_UNIVISION_HW_I2C(const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE)
  : U8G2()
  {
	  /*Page 1，2，3显示，右缺几像素，且浅白色闪烁*/
	  //u8g2_Setup_ssd1306_i2c_128x32_univision_f(&u8g2, rotation, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);	//测试OK

	  /*Page 1，2，3显示，右缺几像素，无闪烁*/
	  //HWI2C//u8g2_Setup_ssd1306_i2c_128x32_univision_1(&u8g2, rotation, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);	//测试OK
	  /*DMA*/
	  //u8g2_Setup_ssd1306_i2c_128x32_univision_1(&u8g2, rotation, u8x8_byte_stm32hal_hw_i2c_dma, psoc_gpio_and_delay_cb);

#ifdef OLED_I2C_NONE_DMA
	  u8g2_Setup_ssd1306_i2c_128x32_univision_2(&u8g2, rotation, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);	//测试OK
#endif

#ifdef OLED_I2C_WITH_DMA
	  u8g2_Setup_ssd1306_i2c_128x32_univision_f(&u8g2, rotation, u8x8_byte_stm32hal_hw_i2c_dma, u8x8_gpio_and_delay); // 初始化 u8g2 结构体
	  //u8g2_Setup_ssd1316_128x32_f(&u8g2, rotation, u8x8_byte_stm32hal_hw_i2c_dma, u8x8_gpio_and_delay);
	  #endif
  }
};

extern U8G2_SSD1306_128X32_UNIVISION_HW_I2C u8g2;
}
#endif

#endif // __OLED_INIT_H

