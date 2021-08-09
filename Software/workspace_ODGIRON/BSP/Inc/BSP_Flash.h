/*
 * BSP_Flash.h
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 */
#include "stdint.h"
#ifndef BSP_BSP_FLASH_H_
#define BSP_BSP_FLASH_H_
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Wrappers to allow read/writing to a sector of flash that we use to store all of the user settings
 *
 * Should allow reading and writing to the flash
 */
#include "main.h"
#ifdef STM32F4
#define FLASH_PROGRAM_TIME (uint16_t)50
#define FLASH_ERASE_TIME (uint16_t)50
//F401CCU6 Sector地址
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */

#endif
#ifdef __cplusplus
}
#endif
#endif /* BSP_BSP_FLASH_H_ */
