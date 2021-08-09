/*

  u8x8_d_ssd1306_128x32.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
*/


#include "u8x8.h"



/* UG-2832HSWEG02 Datasheet, Section 4.4 */
static const uint8_t u8x8_d_ssd1306_128x32_univision_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
#if 1
	U8X8_C(0xAE), /*display off*/

	/*triple byte command 三字节命令，CH1115没有该命令的第一个命令，而仅有后两个*/
	U8X8_C(0x21), 	//Set Column Address (21h): 127-0 = 127
	U8X8_C(0x00),		//Column start address: 0
	U8X8_C(0x7F),		//Column end address: 127

	U8X8_C(0x22),		//Set Page Address (22h)，Set the page start and end address of the target display location by command 22h
	U8X8_C(0x00),		//Page start address
	U8X8_C(0x03),		//Page end address

	U8X8_C(0xD5), /*set osc division*///选择内部振荡器时，可以通过命令D5h A [7：4]更改其输出频率
	//U8X8_C(0x80), /*Divide ratios*//1000，0000
	//U8X8_C(0xC1), /*Divide ratios*///0101,0010 5  2  100Hz
	U8X8_C(0xC0), //DEFAULT: 1100,0001 12  1   200Hz

	U8X8_C(0xA8), /*multiplex ratio*/
	U8X8_C(0x1F), /*0x1F = 0~31 ,即COM0~31 duty = 1/32*/

	U8X8_C(0xC8), /*Com scan direction 0XC0 */

	U8X8_C(0xD3), /*set vertical display offset*/
	U8X8_C(0x00), /* 0x20 */

	U8X8_C(0x40), /*set display start line*///(40h~66h)from 0 to 38

	U8X8_C(0xA0), /*set segment remap 0XA0*/

	U8X8_C(0x8d), /*set vcomh 即Charge Pump*/
	U8X8_C(0x15), /* 设置电压 0x14:9v; 0x15 7.5v */

	U8X8_C(0xDA), /*set COM pins*/
	U8X8_C(0x12), //0001,0010 A[4]Oddeven (1) / Sequential (0) Left / Right Swap A[5]

	//对比度0%	//熄屏
	U8X8_C(0x81), /*contract control*/
	U8X8_C(0x00), /*256*/

	U8X8_C(0xD9), /*set pre-charge period*/
	U8X8_C(0x22), /*Pre charge period*/

	U8X8_C(0xDB), /*set vcomh*///内部稳压VCOMH或外部VCOMH
	U8X8_C(0x20), //Set VCOMH Deselect Level: 00h:0.65 x VCC 20h:0.77 x VCC (RESET) 30h:0.83 x VCC

	//根据GDDRAM内容，A4h命令使能显示输出。如果发出了A5h命令，则通过使用A4h命令，显示将恢复到GDDRAM内容。换句话说，A4h命令从整个显示“ ON”阶段恢复显示。无论显示数据RAM的内容如何，​​A5h命令都将整个显示器强制为“ ON”。
	U8X8_C(0xA4), /*Enable the display GDDR*/

	//正常显示模式，RAM数据为1表示“ ON”像素，
	U8X8_C(0xA6), /*normal / reverse*///正常显示模式，RAM数据为1表示“ ON”像素，

	U8X8_C(0x20), /*Memory Mode*///Set Memory Addressing Mode (20h) ..
	U8X8_C(0x00), /*Wrap memory*/

	//OLED_Clear默认要熄屏，上电才不会有雪花乱码，之后主程序里先清显存 再调高亮度
	U8X8_C(0xAE), /*display OFF:Sleep mode*/
#elif 1
  U8X8_C(0x0ae),		                /* display off */
  U8X8_CA(0x0d5, 0x080),		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
  U8X8_CA(0x0a8, 0x01f),		/* multiplex ratio */
  U8X8_CA(0x0d3, 0x000),		/* display offset */
  U8X8_C(0x040),		                /* set display start line to 0 */
  U8X8_CA(0x08d, 0x014),		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable */
  U8X8_CA(0x020, 0x000),		/* horizontal addressing mode */
  
  U8X8_C(0x0a1),				/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  // Flipmode
  // U8X8_C(0x0a0),				/* segment remap a0/a1*/
  // U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  
  U8X8_CA(0x0da, 0x002),		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
  U8X8_CA(0x081, 0x08f),		/* [2] set contrast control */
  U8X8_CA(0x0d9, 0x0f1),		/* [2] pre-charge period 0x022/f1*/
  U8X8_CA(0x0db, 0x040),		/* vcomh deselect level */
  
  U8X8_C(0x02e),				/* Deactivate scroll */ 
  U8X8_C(0x0a4),				/* output ram to display */
  U8X8_C(0x0a6),				/* none inverted normal display mode */

#else 	//写入起始坐标正常了，但只显示了一个page
  	U8X8_C(0xAE), /*display off*/
	U8X8_C(0x00), /*set lower column address*/
	U8X8_C(0x10), /*set higher column address*/
	U8X8_C(0x40), /*set display start line*/
	U8X8_C(0xB0), /*set page address*/
	U8X8_C(0x81), /*contract control*/
	U8X8_C(0x4D), /*128*/
	U8X8_C(0x82), //*IREF Resister Set: (82H)*/
	U8X8_C(0x00), //*Contrast Data Register Set: (00H – FFH)*/
	U8X8_C(0xA1), /*. Set Segment Re-map: (A0H - A1H)*/
	U8X8_C(0xA2), /*Set SEG pads hardware configuration: (A2H – A3H)*/
	U8X8_C(0xA4), //*Set Entire Display OFF/ON: (A4H - A5H),整个显示器开关/
	U8X8_C(0xA6), /*normal / reverse*/
	U8X8_C(0xA8), /*multiplex ratio*/
	U8X8_C(0x1F), /*duty = 1/32*/
	U8X8_C(0xC8), /*Com scan direction*/
	U8X8_C(0xD3), /*set display offset*/
	U8X8_C(0x10),
	U8X8_C(0xD5), /*set osc division*/
	U8X8_C(0xB1),
	U8X8_C(0xD9), /*set pre-charge period*/
	U8X8_C(0x62),
	U8X8_C(0xDB), /*set COM pins 决定oled点阵硬件布局*/
	U8X8_C(0x3F),
	U8X8_C(0xdb), /*set vcomh*/
	U8X8_C(0x3F),
	U8X8_C(0x32), /*set charge pump enable*/
	U8X8_C(0xAD),
	U8X8_C(0x8B),
	U8X8_C(0xAF),
#endif
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x32_univision_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                //适用CH1115/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x32_univision_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                //适用CH1115/* display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x32_univision_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a1),				//段重映射a1/* segment remap a0/a1*/
  U8X8_C(0x0c8),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1306_128x32_univision_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0a0),				//段重映射a0/* segment remap a0/a1*/
  U8X8_C(0x0c0),				/* c0: scan dir normal, c8: reverse */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_ssd1306_128x32_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x32_univision_display_info);
      break;
    */
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x32_univision_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x32_univision_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x32_univision_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )/*段重映射flip0和flip1不会对仅显示page0有任何改变*/
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x32_univision_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1306_128x32_univision_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1306 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 8;
      x += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (((u8x8_tile_t *)arg_ptr)->y_pos));
      
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	u8x8_cad_SendData(u8x8, c*8, ptr); 	/* note: SendData can not handle more than 255 bytes */
	/*
	do
	{
	  u8x8_cad_SendData(u8x8, 8, ptr);
	  ptr += 8;
	  c--;
	} while( c > 0 );
	*/
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_ssd1306_128x32_univision_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 4,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 128,
  /* pixel_height = */ 32
};

uint8_t u8x8_d_ssd1306_128x32_univision(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x32_univision_display_info);
      return 1;
    }
    return u8x8_d_ssd1306_128x32_generic(u8x8, msg, arg_int, arg_ptr);
}

/*=============================================*/
/* issue 756 */

#define	    ADDR_MODE	0 //0:horizontal, 1:vertical, 2:page


static const u8x8_display_info_t u8x8_ssd1306_128x32_winstar_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 4,
  /* default_x_offset = */ 125,
  /* flipmode_x_offset = */ 125,
  /* pixel_width = */ 128,
  /* pixel_height = */ 32
};

uint8_t u8x8_d_ssd1306_128x32_winstar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1306_128x32_winstar_display_info);
      return 1;
    }
    return u8x8_d_ssd1306_128x32_generic(u8x8, msg, arg_int, arg_ptr);
}


/*=============================================*/
/* visionox 132x32 OLED, https://github.com/olikraus/u8g2/issues/1250 */


static const uint8_t u8x8_d_sh1106_128x32_visionox_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_C(0xAE),
  U8X8_CA(0xD5, 0x91),
  U8X8_CA(0xA8, 0x1F),
  U8X8_CA(0xD3, 0x10),
  U8X8_C(0x40),
  U8X8_CA(0xAD, 0x8B),
  U8X8_C(0x33),
  U8X8_C(0xA1),
  U8X8_C(0xC8),
  U8X8_CA(0xDA, 0x12),
  U8X8_CA(0x81, 0xAF),
  U8X8_CA(0xD9, 0x1F),
  U8X8_CA(0xDB, 0x25),
  U8X8_C(0xA4),
  U8X8_C(0xA6),
    
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_d_sh1106_128x32_visionox_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1306: 3 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1306: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1306: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	/* SSD1306: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,
  /* tile_hight = */ 4,
  /* default_x_offset = */ 2,
  /* flipmode_x_offset = */ 2,
  /* pixel_width = */ 128,
  /* pixel_height = */ 32
};


uint8_t u8x8_d_sh1106_128x32_visionox(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
    {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_d_sh1106_128x32_visionox_display_info);
      return 1;
    }

    if ( msg == U8X8_MSG_DISPLAY_INIT )
    {
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_sh1106_128x32_visionox_init_seq);    
    }
    
    return u8x8_d_ssd1306_128x32_generic(u8x8, msg, arg_int, arg_ptr);
}
