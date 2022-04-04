/*
 * FRToSI2C.cpp
 *
 *  Created on: 14Apr.,2018
 *      Author: Ralim
 *      Modify: OldGerman
 */
#include "I2C_Wrapper.h"
#include <Threads.hpp>
#include "BSP.h"
#include "Setup.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif

//#include "OLED.hpp"
FRToSI2C FRToSI2C1(&hi2c1);

#ifdef STM32F1
FRToSI2C FRToSI2C2(&hi2c2);
#endif
#ifdef STM32F4
FRToSI2C FRToSI2C2(&hi2c3);	//同样叫FRToSI2C2，但是改为hi2c3，这样INA，LIS3DH，FUSB302不受影响
#endif
uint8_t oledPage = 0;

SemaphoreHandle_t FRToSI2C::_I2CSemaphore2 = nullptr;	//用于FUSB302
StaticSemaphore_t FRToSI2C::_xSemaphoreBuffer2; 		//用于FUSB302
void FRToSI2C::FRToSInit() {
	if (_I2CSemaphore == nullptr) {
		//_I2CSemaphore = xSemaphoreCreateBinaryStatic(&(_xSemaphoreBuffer));
		_I2CSemaphore = xSemaphoreCreateMutexStatic(&(_xSemaphoreBuffer));
		unlock(); //同xSemaphoreGive(_I2CSemaphore);//释放信号量
	}
	if (_I2CSemaphore2 == nullptr) {
		_I2CSemaphore2 = xSemaphoreCreateMutexStatic(&(_xSemaphoreBuffer2));
		unlock2(); //同xSemaphoreGive(_I2CSemaphore);//释放信号量
	}
}

/*
 * 被I2C非阻塞模式(Interrupt and DMA) 的回调函数调用
 * 功能：释放I2C信号量
 * 唯一使用场景：HAL_GPIO_EXTI_Callback()
 * 原子FreeRTOS指南 P236
 */
void FRToSI2C::CpltCallback(I2C_HandleTypeDef *I2C_Handle) {
	//强制状态重置（即使发送错误）
	// Force state reset (even if tx error)
	I2C_Handle->State = HAL_I2C_STATE_READY;
	if ((I2C_Handle == FRToSI2C1.getI2C_Handle())
			&& (*FRToSI2C1.getI2CSemaphore()))
		xSemaphoreGiveFromISR(*FRToSI2C1.getI2CSemaphore(), NULL);

	else if ((I2C_Handle == FRToSI2C2.getI2C_Handle())
			&& (*FRToSI2C2.getI2CSemaphore()))
		xSemaphoreGiveFromISR(*FRToSI2C2.getI2CSemaphore(), NULL);//稍后任务能获取到中断回调函数释放信号量，解除任务的阻塞态
	else
		;
}

/**
 * @brief  阻塞模式下读取大量数据
 * 		   使用场景：
 * @param  DevAddress:  Target device address: The device 7 bits address value
 *         				in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress:  Internal memory address
 * @param  pData: 		Pointer to data buffer
 * @param  Size: 		Amount of data to be sent
 * @retval bool
 */
bool FRToSI2C::Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
		uint8_t *pData, uint16_t Size) {

	if (!lock())		//尝试获取_I2CSemaphore
		return false;

	if (HAL_I2C_Mem_Read(_I2C_Handle, DevAddress, MemAddress,
			I2C_MEMADD_SIZE_8BIT, pData, Size, 500) != HAL_OK) {

		I2C_Unstick();	//传输出错会执行
		unlock();		//释放_I2CSemaphore
		return false;
	}

	unlock();			//释放_I2CSemaphore
	return true;
}

/**
 * @brief  阻塞模式下向1个寄存器写入1个8bit数据
 * 		   使用场景：在FRToSI2C::writeRegistersBulk()内用于传输
 * 		   			 FRToSI2C::I2C_REG结构体数组的数据
 * @param  address:  I2C从设备地址
 * @param  reg: 	 寄存器地址
 * @param  data:     向寄存器写入的值
 * @retval bool
 */
bool FRToSI2C::I2C_RegisterWrite(uint8_t address, uint8_t reg, uint8_t data) {
	return Mem_Write(address, reg, &data, 1);	//传输1个8bit数据
}

/**
 * @brief  阻塞模式下从1个寄存器读出1个8bit数据
 * 		   使用场景：
 * @param  add: 		I2C从设备地址
 * @param  reg: 		寄存器地址
 * @retval 8bit寄存器值
 */
uint8_t FRToSI2C::I2C_RegisterRead(uint8_t add, uint8_t reg) {
	uint8_t tx_data[1];
	Mem_Read(add, reg, tx_data, 1);
	return tx_data[0];
}

/**
 * @brief  阻塞模式下发送大量数据
 * 		   使用场景：在FRToSI2C::I2C_RegisterWrite()内用于传输1个8bit数据
 * @param  DevAddress:  Target device address: The device 7 bits address value
 *         				in datasheet must be shifted to the left before calling the interface
 * @param  MemAddress:  Internal memory address
 * @param  pData: 		Pointer to data buffer
 * @param  Size: 		Amount of data to be sent
 * @retval bool
 */
bool FRToSI2C::Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
		uint8_t *pData, uint16_t Size) {

	if (!lock())
		return false;
	if (HAL_I2C_Mem_Write(_I2C_Handle, DevAddress, MemAddress,
			I2C_MEMADD_SIZE_8BIT, pData, Size, 500) != HAL_OK) {

		I2C_Unstick();
		unlock();
		return false;
	}

	unlock();
	return true;
}

/**
 * @brief  非阻塞模式下使用DMA在发送大量数据
 * 		   唯一使用场景：OLED::refresh()
 * @param  DevAddress: 	I2C从设备地址
 * @param  pData: 		SRAM中待传输的数据地址
 * @param  Size: 		数据大小
 * @retval bool
 */
bool FRToSI2C::Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {
	if (!lock()) {
		usb_printf("locked! Can't Transmit\r\n");
		return false;
	}
	if (HAL_I2C_Master_Transmit_DMA(_I2C_Handle, DevAddress, pData, Size)
			!= HAL_OK) {
		I2C_Unstick();
		unlock();
		usb_printf("!= HAL_OK, Can't Transmit\r\n");
		return false;
	}

	//HAL_Delay(1);//u8g DMA使用了HAL delay
	usb_printf("Transmit successed!\r\n");
	return true;
}

/**
 * @brief  非阻塞模式下使用DMA接受大量数据
 * 		   唯一使用场景：OLED::refresh()
 * @param  DevAddress: 	I2C从设备地址
 * @param  pData: 		SRAM中待传输的数据地址
 * @param  Size: 		数据大小
 * @retval bool
 */
void FRToSI2C::Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {

}

bool FRToSI2C::Master_Transmit(uint16_t DevAddress, uint8_t *pData,
		uint16_t Size) {
	if (!lock()) {
		usb_printf("locked! Can't Transmit\r\n");
		return false;
	}
	if (HAL_I2C_Master_Transmit(_I2C_Handle, DevAddress, pData, Size,
			HAL_MAX_DELAY) != HAL_OK) {
		I2C_Unstick();
		unlock();
		usb_printf("!= HAL_OK, Can't Transmit\r\n");
		return false;
	}

	//HAL_Delay(1);//u8g DMA使用了HAL delay
	usb_printf("Transmit successed!\r\n");
	return true;
}

bool FRToSI2C::Master_Receive(uint16_t DevAddress, uint8_t *pData,
		uint16_t Size) {
	if (!lock()) {
		usb_printf("locked! Can't Transmit\r\n");
		return false;
	}
	if (HAL_I2C_Master_Receive(_I2C_Handle, DevAddress, pData, Size,
			HAL_MAX_DELAY) != HAL_OK) {
		I2C_Unstick();
		unlock();
		usb_printf("!= HAL_OK, Can't Transmit\r\n");
		return false;
	}

	//HAL_Delay(1);//u8g DMA使用了HAL delay
	usb_printf("Transmit successed!\r\n");
	return true;
}

/**
 * @brief  探测I2C1指定地址上有没有设备
 * @param  DevAddress: Target device address: The device 7 bits address value
 *         in datasheet must be shifted to the left before calling the interface
 * @retval bool
 */
bool FRToSI2C::probe(uint16_t DevAddress) {
	//若获取_I2CSemaphore失败
	if (!lock())
		return false;	//返回false，终止本函数

	//若成功获取_I2CSemaphore则执行以下代码
	uint8_t buffer[1];
	bool worked = HAL_I2C_Mem_Read(_I2C_Handle, DevAddress, 0x0F,
			I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000) == HAL_OK;
	unlock();
	return worked;
}

//若I2C1出错会使用此方法终止当前传输, 并重置I2C
//典型使用场景：if(HAL_I2C通信函数!= HAL_OK)，执行I2C_Unstick()
void FRToSI2C::I2C_Unstick() {
	unstick_I2C(_I2C_Handle);
}

//释放_I2CSemaphore (前提FRToI2C::lock成功获取_I2CSemaphore)
void FRToSI2C::unlock() {
	xSemaphoreGive(_I2CSemaphore);
}

//尝试获取_I2CSemaphore
bool FRToSI2C::lock() {
	bool mark = 0;
	mark = (xSemaphoreTake(_I2CSemaphore, (TickType_t)50) == pdTRUE);
	return mark;
}

/**
 * @brief  向I2C设备写入FRToSI2C::I2C_REG结构体数组
 * 		   典型场景：初始化I2C设备配置多个寄存器
 * @param  address: 		I2C从设备地址
 * @param  registers: 		数组地址
 * @param  registersLength: 数组元素大小(sizeof()传入)
 * @retval bool
 */
bool FRToSI2C::writeRegistersBulk(const uint8_t address,
		const I2C_REG *registers, const uint8_t registersLength) {
	for (int index = 0; index < registersLength; index++) {
		//I2C_RegisterWrite()在阻塞模式下向1个寄存器写入1个8bit数据
		if (!I2C_RegisterWrite(address, registers[index].reg,
				registers[index].val)) {
			return false;
		}
		if (registers[index].pause_ms)
			delay_ms(registers[index].pause_ms);
	}
	return true;
}

void FRToSI2C::unlock2() {
	xSemaphoreGive(_I2CSemaphore2);
}

bool FRToSI2C::lock2() {
	if (_I2CSemaphore2 == NULL) {
		asm("bkpt");
	}
	bool a = xSemaphoreTake(_I2CSemaphore2, (TickType_t)500) == pdTRUE;

	return a;
}
