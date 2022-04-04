/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_dfu_if.c
 * @brief          : Usb device for Download Firmware Update.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_DFU
  * @brief Usb DFU device module.
  * @{
  */

/** @defgroup USBD_DFU_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Defines
  * @brief Private defines.
  * @{
  */

#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/03*016Ka,01*016Kg,01*064Kg,07*128Kg"

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
void  Flash_Init_FS(void);
void Flash_DeInit_FS(void);
uint16_t Flash_Erase_FS(uint32_t Add);
uint16_t Flash_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
uint8_t* Flash_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
uint32_t Flash_GetSector(uint32_t Address);
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DFU_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static uint16_t MEM_If_Init_FS(void);
static uint16_t MEM_If_Erase_FS(uint32_t Add);
static uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t MEM_If_DeInit_FS(void);
static uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif
__ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_FS __ALIGN_END =
{
   (uint8_t*)FLASH_DESC_STR,
    MEM_If_Init_FS,
    MEM_If_DeInit_FS,
    MEM_If_Erase_FS,
    MEM_If_Write_FS,
    MEM_If_Read_FS,
    MEM_If_GetStatus_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Memory initialization routine.
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init_FS(void)
{
  /* USER CODE BEGIN 0 */
	HAL_FLASH_Unlock();
	  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes Memory
  * @retval USBD_OK if operation is successful, MAL_FAIL else
  */
uint16_t MEM_If_DeInit_FS(void)
{
  /* USER CODE BEGIN 1 */
	HAL_FLASH_Lock();

	return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Erase_FS(uint32_t Add)
{
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
uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
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
uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
{
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

/**
  * @brief  Get status routine
  * @param  Add: Address to be read from
  * @param  Cmd: Number of data to be read (in bytes)
  * @param  buffer: used for returning the time necessary for a program or an erase operation
  * @retval USBD_OK if operation is successful
  */
uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  /* USER CODE BEGIN 5 */
	switch (Cmd) {
	case DFU_MEDIA_PROGRAM:
		buffer[1] = (uint8_t) FLASH_PROGRAM_TIME;
		buffer[2] = (uint8_t) (FLASH_PROGRAM_TIME << 8);
		buffer[3] = 0;
		break;

	case DFU_MEDIA_ERASE:
		buffer[1] = (uint8_t) FLASH_ERASE_TIME;
		buffer[2] = (uint8_t) (FLASH_ERASE_TIME << 8);
		buffer[3] = 0;
	default:

		break;
	}
	return (USBD_OK);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
void  Flash_Init_FS(void){
	MEM_If_Init_FS();
}
void Flash_DeInit_FS(void){
	MEM_If_DeInit_FS();
}
uint16_t Flash_Erase_FS(uint32_t Add){
	return MEM_If_Erase_FS(Add);
}
uint16_t Flash_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len){
	return MEM_If_Write_FS(src, dest, Len);
}
uint8_t* Flash_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len){
	return MEM_If_Read_FS(src, dest, Len);
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

void USB_Status_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11 | GPIO_PIN_12, GPIO_PIN_RESET);	//挂起

    /*Configure GPIO pin : USB*/								//重新设置
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //假如不行的话，下面的延时加长即可�????????
    //HAL_Delay(1000);															//故意等一等，让上位机发现USB设备更改
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
