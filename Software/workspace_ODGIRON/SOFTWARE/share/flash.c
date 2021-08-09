/*
 * flash.c
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 *      Modify: OldGerman
 */
#include "Settings.h"
#include "main.h"
#include "flash.h"
#ifndef ODGIRON_BOOTLDR
#include "BSP.h"
#include "BSP_Flash.h"
#else
#include "usbd_dfu_if.h"
#endif

#ifdef STM32F1
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#endif

#include "string.h"
extern uint16_t settings_page[512];


/* @brief  Memory read routine.
* @param  src: Pointer to the source buffer. Address to be written to.
* @param  dest: Pointer to the destination buffer.
* @param  Len: Number of data to be read (in bytes).
* @retval Pointer to the physical address where data should be read.
*/
//uint8_t* (uint8_t *src, uint8_t *dest, uint32_t Len)
void flash_read_buffer(uint8_t *buffer, const uint16_t length) {
	memcpy(buffer, settings_page, length);
	//Flash_Read_FS(buffer, (uint8_t*)settings_page, length);
}

#ifdef STM32F4
/*FLASH操作函数，与BOOTLDR工程usbd_dfu_if.h的一致*/
#ifndef ODGIRON_BOOTLDR
void  Flash_Init_FS(void);
void Flash_DeInit_FS(void);
static uint16_t Flash_Erase_FS(uint32_t Add);
static uint16_t Flash_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t* Flash_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint32_t Flash_GetSector(uint32_t Address);
#endif
#endif

uint8_t flash_save_buffer(uint8_t *buffer, const uint16_t length) {

#ifdef STM32F1
  FLASH_EraseInitTypeDef pEraseInit;
  pEraseInit.TypeErase    = FLASH_TYPEERASE_PAGES;
  pEraseInit.Banks        = FLASH_BANK_1;
  pEraseInit.NbPages      = 1;
  pEraseInit.PageAddress  = (uint32_t)settings_page;
  uint32_t failingAddress = 0;
  resetWatchdog();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);
  HAL_FLASH_Unlock();
  HAL_Delay(1);
  resetWatchdog();
  HAL_FLASHEx_Erase(&pEraseInit, &failingAddress);
  //^ Erase the page of flash (1024 bytes on this stm32)
  // erased the chunk
  // now we program it
  uint16_t *data = (uint16_t *)buffer;
  HAL_FLASH_Unlock();
  for (uint8_t i = 0; i < (length / 2); i++) {
    resetWatchdog();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&settings_page[i], data[i]);
  }
  HAL_FLASH_Lock();
#elif defined(STM32F4)

  resetWatchdog();
  /*flash的初始化，解锁flash和清除一些flash的异常状态标识*/
  Flash_Init_FS();

  HAL_Delay(1);
  resetWatchdog();
  Flash_Erase_FS(0x0800C000);


  //^ Erase the page of flash (1024 bytes on this stm32)
  // erased the chunk
  resetWatchdog();
  Flash_Write_FS(buffer, (uint8_t*)settings_page, length);

  resetWatchdog();
  Flash_DeInit_FS();

#if 0
  // now we program it
  uint16_t *data = (uint16_t *)buffer;
  HAL_FLASH_Unlock();
  for (uint8_t i = 0; i < (length / 2); i++) {
    resetWatchdog();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&settings_page[i], data[i]);
  }
  HAL_FLASH_Lock();
#endif
#endif
  return 1;
}
#ifdef STM32F4
#ifndef ODGIRON_BOOTLDR

/**
 * @brief  Memory initialization routine.
 * @retval USBD_OK if operation is successful, MAL_FAIL else.
 */
void Flash_Init_FS(void) {
	/* USER CODE BEGIN 0 */
	HAL_FLASH_Unlock();
	  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	/* USER CODE END 0 */
}

/**
 * @brief  De-Initializes Memory
 * @retval USBD_OK if operation is successful, MAL_FAIL else
 */
void Flash_DeInit_FS(void) {
	/* USER CODE BEGIN 1 */
	HAL_FLASH_Lock();
	/* USER CODE END 1 */
}

/**
 * @brief  Erase sector.
 * @param  Add: Address of sector to be erased.
 * @retval 0 if operation is successful, MAL_FAIL else.
 */
uint16_t Flash_Erase_FS(uint32_t Add) {
	/* USER CODE BEGIN 2 */
	uint32_t startsector = 0;
	uint32_t sectornb = 0;
	/* Variable contains Flash operation status */
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef eraseinitstruct;

	/* Get the number of sector */
	startsector = Flash_GetSector(Add);

	eraseinitstruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	eraseinitstruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	eraseinitstruct.Sector = startsector;
	eraseinitstruct.NbSectors = 1;
	status = HAL_FLASHEx_Erase(&eraseinitstruct, &sectornb);

	if (status != HAL_OK) {
		return 1;
	}
	return 0;
	//return (USBD_OK);
	/* USER CODE END 2 */
}

/**
 * @brief  Memory write routine.
 * @param  src: Pointer to the source buffer. Address to be written to.
 * @param  dest: Pointer to the destination buffer.
 * @param  Len: Number of data to be written (in bytes).
 * @retval USBD_OK if operation is successful, MAL_FAIL else.
 */
uint16_t Flash_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len) {

	/* USER CODE BEGIN 3 */
	uint32_t i = 0;

	for (i = 0; i < Len; i += 4) {
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
		 be done by byte */
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) (dest + i),
				*(uint32_t*) (src + i)) == HAL_OK) {
			/* Check the written value */
			if (*(uint32_t*) (src + i) != *(uint32_t*) (dest + i)) {
				/* Flash content doesn't match SRAM content */
				return 2;
			}
		} else {
			/* Error occurred while writing data in Flash memory */
			return 1;
		}
	}
	return 0;
	//return (USBD_OK);
	/* USER CODE END 3 */
}

/**
 * @brief  Memory read routine.
 * @param  src: Pointer to the source buffer. Address to be written to.
 * @param  dest: Pointer to the destination buffer.
 * @param  Len: Number of data to be read (in bytes).
 * @retval Pointer to the physical address where data should be read.
 */
uint8_t* Flash_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len) {
	/* Return a valid address to avoid HardFault */
	/* USER CODE BEGIN 4 */
	uint32_t i = 0;
	uint8_t *psrc = src;

	for (i = 0; i < Len; i++) {
		dest[i] = *psrc++;
	}
	/* Return a valid address to avoid HardFault */
	return (uint8_t*) (dest);
	//return (uint8_t*) (USBD_OK);
	/* USER CODE END 4 */
}



uint32_t Flash_GetSector(uint32_t Address) {
	uint32_t sector = 0;
	if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
		sector = FLASH_SECTOR_0;
	} else if ((Address < ADDR_FLASH_SECTOR_2)
			&& (Address >= ADDR_FLASH_SECTOR_1)) {
		sector = FLASH_SECTOR_1;
	} else if ((Address < ADDR_FLASH_SECTOR_3)
			&& (Address >= ADDR_FLASH_SECTOR_2)) {
		sector = FLASH_SECTOR_2;
	} else if ((Address < ADDR_FLASH_SECTOR_4)
			&& (Address >= ADDR_FLASH_SECTOR_3)) {
		sector = FLASH_SECTOR_3;
	} else if ((Address < ADDR_FLASH_SECTOR_5)
			&& (Address >= ADDR_FLASH_SECTOR_4)) {
		sector = FLASH_SECTOR_4;
	} else {
		sector = FLASH_SECTOR_5;
	}
	return sector;
}
#endif
#endif
