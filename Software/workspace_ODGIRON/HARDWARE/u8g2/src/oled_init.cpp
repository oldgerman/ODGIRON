/*
 * oled_init.cpp
 *
 *  Created on: Jan 17, 2021
 *      Author: 29209
 */


#include "oled_init.h"
#include "main.h"
#ifndef ODGIRON_BOOTLDR
#include "cmsis_os.h"
#endif
U8G2_SSD1306_128X32_UNIVISION_HW_I2C u8g2(U8G2_R0);


#ifdef OLED_I2C_NONE_DMA
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
	static uint8_t buffer[32];
	static uint8_t buf_idx;
	uint8_t *data;

	switch(msg)
	{
	case U8X8_MSG_BYTE_SEND:
		data = (uint8_t *)arg_ptr;
		while( arg_int > 0 )
		{
			buffer[buf_idx++] = *data;
			data++;
			arg_int--;
		}
		break;
	case U8X8_MSG_BYTE_INIT:
		/* add your custom code to init i2c subsystem */
		break;
	case U8X8_MSG_BYTE_SET_DC:
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		buf_idx = 0;
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		if(HAL_I2C_Master_Transmit(&hi2c1, (DEVICE_ADDRESS << 1), buffer, buf_idx, TX_TIMEOUT) != HAL_OK) return 0;
		break;
	default:
		return 0;
	}
	return 1;
}
#endif

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	/* STM32 supports HW SPI, Remove unused cases like U8X8_MSG_DELAY_XXX & U8X8_MSG_GPIO_XXX */
	switch(msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT:
		/* Insert codes for initialization */
		break;
	case U8X8_MSG_DELAY_MILLI:
		/* ms Delay */
		//HAL_Delay(arg_int);
		HAL_Delay(1);
		break;
	}
	return 1;
}

void u8g2Init(u8g2_t *u8g2)
{
#ifdef OLED_I2C_NONE_DMA
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay); // 初始化 u8g2 结构体
#endif

#ifdef OLED_I2C_WITH_DMA
	//u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_stm32hal_hw_i2c_dma, u8x8_gpio_and_delay); // 初始化 u8g2 结构体
    //u8g2_Setup_ssd1316_128x32_1(u8g2, U8G2_R0, u8x8_byte_stm32hal_hw_i2c_dma, u8x8_gpio_and_delay);

    #endif

	u8g2_InitDisplay(u8g2);                                                                       // 根据所选的芯片进行初始化工作，初始化完成后，显示器处于关闭状态
    u8g2_SetPowerSave(u8g2, 0);                                                                   // 打开显示器
    u8g2_ClearBuffer(u8g2);
}

void u8g2_begin(){
	u8g2.begin();
}

#if 1
//https://qiita.com/tlab/items/e62d76a23a1496654e09
uint8_t u8x8_byte_stm32hal_hw_i2c_dma(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32];      /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    //usb_printf("msg = %d   " , msg);
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
    {
        data = (uint8_t *)arg_ptr;
        while( arg_int > 0 )
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
    }
        break;
    case U8X8_MSG_BYTE_INIT:
        break;
    case U8X8_MSG_BYTE_SET_DC:
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        {
            buf_idx = 0;
        }
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
    {
        uint8_t iaddress = DEVICE_ADDRESS;	//0x3c
        //usb_printf("buf_indx = %d\r\n", buf_idx);
      if(HAL_I2C_Master_Transmit_DMA(&hi2c1, (uint16_t)iaddress<<1, &buffer[0], buf_idx)!= HAL_OK)
    	  return 0;
#ifndef ODGIRON_BOOTLDR
      osDelay(1);
#else
       HAL_Delay(3);
#endif


    }
        break;
    default:
      return 0;
  }
  return 1;
}

#endif
