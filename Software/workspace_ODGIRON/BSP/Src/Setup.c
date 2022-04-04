/*
 * Setup.c
 *
 *  Created on: 29Aug.,2017
 *      Author: Ben V. Brown
 */
#include "Setup.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif

uint32_t ADCReadings[ADC_CHANNELS * ADC_SAMPLES] = {0}; // room for 32 lots of the pair of readings


/**
  * @brief  获取同步规则模式下的DMA搬到SRAM的32位的ADC1_DR寄存器数据
  * @note	上半个字包含ADC2的转换数据，低半个字包含ADC1的转换数据，ADC1 ADC2数据都是12bit右对齐
  * @note   两个ADC RANK数量、同一RANK号的ADC通道编号 必须相同
  * @note   ironOS为channel 0 -> tmp36 temperature sensor, channel 1 -> VIN
  * @param  若ADC_CHANNELS = 2， 即序列的rank数量为2，那么参数channel必须为0，或1
  * 		例如 ADC1、ADC2的RANK1、RANK2都分别为CH2、CH3，且ADC_SAMPLES=16
  * 			0：获取ADC1和ADC2通道编号较低的数据各16次的总和 (即ADC1的CH2 16次，即ADC2的CH2 也16次)
  * 			1：获取ADC1和ADC2通道编号较高的数据各16次的总和 (即ADC1的CH3 16次，即ADC2的CH3 也16次)
  * 		例如 ADC_CHANNELS = 3，那么也类似
  * @retval 32次/4得到的8次过采样值 (12bitADC模式下有效范围为0~32768)
 */

uint16_t getADC(uint8_t channel) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
		  uint16_t adc1Sample = ADCReadings[channel + (i * ADC_CHANNELS)];	//uint16_t裁剪了32bit的前16bitADC2 CH1数据
		  //uint16_t adc2Sample = ADCReadings[channel + (i * ADC_CHANNELS)] >> 16;
#ifdef STM32F1
		  //其内部
		 uint16_t adc2Sample = ADCReadings[channel + ((channel == 0)?1:(-1)) + (i * ADC_CHANNELS)] >> 16;
    //								  ^本次要读的通道
    sum += (adc1Sample + adc2Sample)/2;
    //if (i == 0)
    	//usb_printf("i = %d  adc1Samp = %d  adc2Samp = %d\r\n" , i, adc1Sample, adc2Sample);
#elif defined(STM32F4)
    sum += (adc1Sample*2);
    //if (i == 0)
    //usb_printf("i = %d  adc1Samp = %d\r\n" , i, adc1Sample);
#endif
  }
  return sum>>2;
}


