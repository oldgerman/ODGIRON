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

#define FLASH_DESC_STR      "@Internal Flash   /0x08000000/36*001Ka,90*001Kg,02*001Kg"

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
void  Flash_Init_FS(void);
void Flash_DeInit_FS(void);
uint16_t Flash_Erase_FS(uint32_t Add);
uint16_t Flash_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
uint8_t* Flash_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
uint32_t Flash_GetSector(uint32_t Address);
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
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);
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
  FLASH_EraseInitTypeDef pEraseInit;
  uint32_t SectorError;
  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.NbPages = 1;
  pEraseInit.PageAddress = Add;
  pEraseInit.Banks = FLASH_BANK_1;
  if(HAL_FLASHEx_Erase(&pEraseInit,&SectorError)!=HAL_OK)
  {
	return (USBD_FAIL);
  }
  return (USBD_OK);
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
  for(i = 0;i < Len;i += 4)
  {
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
	   (uint32_t)(dest + i),*(uint32_t *)(src + i)) == HAL_OK)
	{
	  if(*(uint32_t *)(src + i) != *(uint32_t *)(dest + i))
		  return 2;
	}
	else
	  return 1;
  }
  return (USBD_OK);
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
  for(i = 0;i < Len;i++) dest[i] = *psrc++;
  return dest;
  //return (uint8_t*)(USBD_OK);
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
    HAL_Delay(1000);															//故意等一等，让上位机发现USB设备更改
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
