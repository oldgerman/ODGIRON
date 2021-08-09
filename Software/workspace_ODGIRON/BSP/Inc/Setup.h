/*
 * Setup.h
 *
 *  Created on: 29Aug.,2017
 *      Author: Ben V. Brown
 */

#ifndef SETUP_H_
#define SETUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

uint16_t   getADC(uint8_t channel);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim); // Since the hal header file does not define this one

#ifdef STM32F1
#define ADC_CHANNELS 2
#elif defined(STM32F4)
#define ADC_CHANNELS 1
#endif
#define ADC_SAMPLES  16
extern uint32_t ADCReadings[ADC_CHANNELS * ADC_SAMPLES]; // room for 32 lots of the pair of readings

#ifdef __cplusplus
}
#endif

#endif /* SETUP_H_ */
