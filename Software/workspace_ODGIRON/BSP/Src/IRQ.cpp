/*
 * IRQ.c
 *
 *  Created on: 30 May 2020
 *      Author: Ralim
 */

#include "IRQ.h"
#include "int_n.h"
#include "I2C_Wrapper.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif
#include <stdio.h>	//提供 __unused 宏

bool ADC_Injected_Callback_Mark = false;
/*
 * Catch the IRQ that says that the conversion is done on the temperature
 * readings coming in Once these have come in we can unblock the PID so that it
 * runs again
 * 注入模式的转换完成的中断的回调函数，一旦进入温度读数
 * 我们就可以取消阻止PID，以便它
 * 再次运行
 *
 * 该函数由注入序列完成时触发
 */
/* ADC IRQHandler and Callbacks used in non-blocking modes (Interruption) */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc) {
#if 0
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (hadc == &hadc1) {
	  //getTipRawTemp(true);	//刷新注入通道数据到滤波器中
	  ADC_Injected_Callback_Mark = true;	//注入转换完成，修改标记
	  getTipRawTemp(1);		//与PIDThread同时使用冲突?
	  //未打印，序列时间不够导致复位？
    if (pidTaskNotification) {
      vTaskNotifyGiveFromISR(pidTaskNotification, &xHigherPriorityTaskWoken);	//发送通知，函数 xTaskNotifyGive()的中断版本。
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      //usb_printf("vTaskNotifyGiveFromISR\r\n");
    }
  }
#else
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hadc == &hadc1) {
    if (pidTaskNotification) {
      vTaskNotifyGiveFromISR(pidTaskNotification, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
#endif
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  //测试用
  //usb_printf("HAL_ADC_ConvCpltCallback!\r\n");
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  //测试用
  //usb_printf("HAL_ADC_ConvHalfCpltCallback!\r\n");
}

/*
 * 非阻塞模式（中断和DMA）中使用的I2C IRQHandler和回调（对__weak重写）
 * I2C IRQHandler and Callbacks used in non blocking modes (Interrupt and DMA)
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }	//主接收完成
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }	//主发送完成
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) { FRToSI2C::CpltCallback(hi2c); }


/* 外部GPIO中断组 EXTI9_5_IRQn 的回调函数。Pins.h中定义的加速度计和FUSB的中断共用的回调函数
 * 实际上仅处理FUSB302B的中断
 * #define INT_Movement_Pin          GPIO_PIN_5		//PB5: ACCEL_IN2
 * #define INT_Movement_GPIO_Port    GPIOB
 * #define INT_PD_Pin                GPIO_PIN_9		//PA9: FUSB302 INT pin
 * #define INT_PD_GPIO_Port          GPIOA
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  //将无用变量强制转换为void的方式, 消除编译器警告 与上面的__unused效果相同
  (void)GPIO_Pin;
  //usb_printf("HAL_GPIO_EXTI_Callback-------------------------\r\n");
  InterruptHandler::irqCallback();	//class InterruptHander 专门用于处理GPIO EXTI5_9
}
