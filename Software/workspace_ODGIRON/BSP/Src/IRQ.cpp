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
 * 我们就可以解除PID的任务里由if判断通知是否来的代码块的阻塞，以便它再次运行
 *
 * 该函数由注入序列完成时触发
 */
/* ADC IRQHandler and Callbacks used in non-blocking modes (Interruption) */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (hadc == &hadc1) {
	  //这里ADC中断完成时PID线程会get到这个任务通知，这个直接影响PID计算周期
    if (pidTaskNotification) {
    	// 17.1 任务通知简介 (正点原子STM32 FreeRTOS开发手册)
    	// xTaskNotifyGive(): 发送通知，不带通知值并且不保留接收任务的通知值，此 函数会将接收任务的通知值加一，用于任务中。
    	// vTaskNotifyGiveFromISR(): 发送通知，函数 xTaskNotifyGive()的中断版本
    	// 24.2 任务计数信号量 vTaskNotifyGiveFromISR()实现资源释放，即对计数信号量的数值进行加一操作----安富莱V5 FreeRTOS
      vTaskNotifyGiveFromISR(pidTaskNotification, &xHigherPriorityTaskWoken);
      //					 ^~~~~~~发送给PIDTask
      
      //退出中断后根据xHigherPriorityTaskWoken的值判断是否需要执行任务切换
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
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
