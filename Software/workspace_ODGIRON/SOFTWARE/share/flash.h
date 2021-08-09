#include "main.h"

#ifndef _FLASH_H
#define _FLASH_H

#ifdef __cplusplus
extern "C" {
uint8_t flash_save_buffer(uint8_t *buffer, const uint16_t length);
void flash_read_buffer(uint8_t *buffer, const uint16_t length);

#endif
#ifdef __cplusplus
}
#endif
#endif
