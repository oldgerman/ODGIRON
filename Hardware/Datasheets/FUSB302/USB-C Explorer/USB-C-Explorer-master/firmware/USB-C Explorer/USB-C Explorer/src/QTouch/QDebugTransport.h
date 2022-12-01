/* This source file is part of the ATMEL QTouch Library 5.0.8 */

/*****************************************************************************
 *
 * \file
 *
 * \brief  This file contains the QDebug transport layer API that can be used to
 * transfer data from a Touch Device to QTouch Studio using the QT600
 * USB Bridge.
 *
 *
 * - Userguide:          QTouch Library Peripheral Touch Controller User Guide.
 * - Support email:      www.atmel.com/design-support/
 *
 *
 * Copyright (c) 2013-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 ******************************************************************************/

#ifndef DEBUGTRANSPORT_H_INCLUDED
#define DEBUGTRANSPORT_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/*============================ INCLUDES ======================================*/
#include <parts.h>
#include "touch.h"

#include "touch_api_ptc.h"

/*============================ MACROS ========================================*/

#if DEF_TOUCH_QDEBUG_ENABLE == 1

#define MESSAGE_START   0x1B

#ifdef DEF_TOUCH_QDEBUG_ENABLE_MUTLCAP
#define TX_BUFFER_SIZE (DEF_MUTLCAP_NUM_CHANNELS * 4) + 10
#define RX_BUFFER_SIZE (DEF_MUTLCAP_NUM_CHANNELS * 4) + 10
#else
#define TX_BUFFER_SIZE (DEF_SELFCAP_NUM_CHANNELS * 4) + 10
#define RX_BUFFER_SIZE (DEF_SELFCAP_NUM_CHANNELS * 4) + 10
#endif

#define STATE_IDLE          0
#define STATE_LENGTH1   1
#define STATE_LENGTH2   2
#define STATE_DATA          3

#define INT(a, b)       ((a << 8) | b)

/*============================ GLOBAL VARIABLES ==============================*/
/* ! TX Buffer globals */
extern uint8_t TX_Buffer[TX_BUFFER_SIZE];
extern uint16_t TX_index;

/* ! RX Buffer globals */
extern uint8_t RX_Buffer[RX_BUFFER_SIZE];
extern uint16_t RX_index;

extern uint8_t SequenceL;
extern uint8_t SequenceH;

/*============================ PROTOTYPES ====================================*/

/*! \brief Initialize the send and receive buffers.
 * \note Called from QDebug_Init.
 */
void Init_Buffers(void);

/*! \brief Puts one byte in the Transmit Buffer.
 * \param data: byte to be sent.
 * \note Called from QDebug_Init.
 */
void PutChar(uint8_t data);

/*! \brief Puts two bytes in the Transmit Buffer.
 * \param data: 16bit data to be sent.
 * \note Big Endian. TX_index is post incremented.
 */
void PutInt(uint16_t data);

/*! \brief Get one byte from the Receive Buffer.
 * \return uint8_t: byte received.
 * \note RX_index is post incremented.
 */
uint8_t GetChar(void);

/*! \brief Send the content of the TX_Buffer to the USB Bridge using the
 * interface selected in QDebugSettings.h
 * \note Called from the transmit functions in QDebug.c.
 */
void Send_Message(void);

/*! \brief Executes a master read transmission if TWI is selected as interface.
 * Checks if RX_Buffer has a valid frame
 * \return uint8_t: returns a true or false dependent on whether a valid frame
 * is
 * available or not
 * \note Called from QDebug_ProcessCommands in QDebug.c.
 */
uint8_t Receive_Message(void);

/*! \brief Handles the incoming bytes from the interface selected in
 * QDebugSettings.h and puts the bytes in the RX_Buffer data read by
 * the selected interface
 * \return uint8_t: returns a true if more data must be read,
 * returns a false if the frame is complete
 * \note Used by SPI and TWI receive handlers.
 */
uint8_t RxHandler(uint8_t c);

#endif  /* #if DEF_TOUCH_QDEBUG_ENABLE == 1 */

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
