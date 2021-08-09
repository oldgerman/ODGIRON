/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc1;
extern IWDG_HandleTypeDef hiwdg;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern bool settingsWereReset;
extern uint16_t GUITask_stacksize;
extern uint16_t PIDTask_stacksize;
extern uint16_t MOVTask_stacksize;
extern uint16_t POWTask_stacksize;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TREF_TEMP_ADC1_CHANNEL ADC_CHANNEL_2
#define TEMP_TEMP_ADC1_CHANNEL ADC_CHANNEL_8
#define TIM1_PSC_FAST_PWM 46978
#define TIM1_PSC_SLOW_PWM 65535
#define BUZZER_CHANNEL TIM_CHANNEL_2
#define SCL1_Pin GPIO_PIN_6
#define SDA1_Pin GPIO_PIN_7
#define SCL1_GPIO_Port GPIOB
#define SDA1_GPIO_Port GPIOB
#define SCL3_Pin GPIO_PIN_8
#define SDA3_Pin GPIO_PIN_4
#define SCL3_GPIO_Port GPIOA
#define SDA3_GPIO_Port GPIOB
#define TREF_TEMP_Pin GPIO_PIN_2
#define TREF_TEMP_GPIO_Port GPIOA
#define BUZZER_OUT_Pin GPIO_PIN_7
#define BUZZER_OUT_GPIO_Port GPIOA
#define TIP_TEMP_Pin GPIO_PIN_0
#define TIP_TEMP_GPIO_Port GPIOB
#define IT_FUSB302B_Pin GPIO_PIN_1
#define IT_FUSB302B_GPIO_Port GPIOB
#define IT_FUSB302B_EXTI_IRQn EXTI1_IRQn
#define KEY_OK_Pin GPIO_PIN_15
#define KEY_OK_GPIO_Port GPIOB
#define KEY_B_Pin GPIO_PIN_8
#define KEY_B_GPIO_Port GPIOB
#define KEY_A_Pin GPIO_PIN_9
#define KEY_A_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
