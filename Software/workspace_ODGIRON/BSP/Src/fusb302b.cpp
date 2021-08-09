/*
 * PD Buddy Firmware Library - USB Power Delivery for everyone
 * Copyright 2017-2018 Clayton G. Hobbs
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Model_Config.h"
#ifdef POW_PD
#include "BSP.h"
#include "fusb302b.h"
#include "int_n.h"
#include "pd.h"

/*
 * Read a single byte from the FUSB302B
 *
 * cfg: The FUSB302B to communicate with
 * addr: The memory address from which to read
 *
 * Returns the value read from addr.
 */
static uint8_t fusb_read_byte(uint8_t addr) {
  uint8_t data = 0;
  data = FRToSI2C2.I2C_RegisterRead(FUSB302B_ADDR, addr);
  if (!data) {
    return 0;
  }
  return data;
}

/*
 * Read multiple bytes from the FUSB302B
 *
 * cfg: The FUSB302B to communicate with
 * addr: The memory address from which to read
 * size: The number of bytes to read
 * buf: The buffer into which data will be read
 */
static bool fusb_read_buf(uint8_t addr, uint8_t size, uint8_t *buf) {
	return FRToSI2C2.Mem_Read(FUSB302B_ADDR, addr, buf, size);
}

/*
 * Write a single byte to the FUSB302B
 *
 * cfg: The FUSB302B to communicate with
 * addr: The memory address to which we will write
 * byte: The value to write
 */
static bool fusb_write_byte(uint8_t addr, uint8_t byte) {
	return FRToSI2C2.I2C_RegisterWrite(FUSB302B_ADDR, addr, byte);
}

/*
 * Write multiple bytes to the FUSB302B
 *
 * cfg: The FUSB302B to communicate with
 * addr: The memory address to which we will write
 * size: The number of bytes to write
 * buf: The buffer to write
 */
static bool fusb_write_buf(uint8_t addr, uint8_t size, const uint8_t *buf) {
	return FRToSI2C2.Mem_Write(FUSB302B_ADDR, addr, (uint8_t *)buf, size);
//													^~~~~~~~~~
}

void fusb_send_message(const union pd_msg *msg) {
  if (!FRToSI2C2.lock2()) {
    return;
  }
  /* Token sequences for the FUSB302B */
  static uint8_t       sop_seq[5] = {FUSB_FIFO_TX_SOP1, FUSB_FIFO_TX_SOP1, FUSB_FIFO_TX_SOP1, FUSB_FIFO_TX_SOP2, FUSB_FIFO_TX_PACKSYM};
  static const uint8_t eop_seq[4] = {FUSB_FIFO_TX_JAM_CRC, FUSB_FIFO_TX_EOP, FUSB_FIFO_TX_TXOFF, FUSB_FIFO_TX_TXON};

  /* Take the I2C2 mutex now so there can't be a race condition on sop_seq */
  /* Get the length of the message: a two-octet header plus NUMOBJ four-octet
   * data objects */
  uint8_t msg_len = 2 + 4 * PD_NUMOBJ_GET(msg);

  /* Set the number of bytes to be transmitted in the packet */
  sop_seq[4] = FUSB_FIFO_TX_PACKSYM | msg_len;

  /* Write all three parts of the message to the TX FIFO */
  fusb_write_buf(FUSB_FIFOS, 5, sop_seq);
  fusb_write_buf(FUSB_FIFOS, msg_len, msg->bytes);
  fusb_write_buf(FUSB_FIFOS, 4, eop_seq);

  FRToSI2C2.unlock2();
}

uint8_t fusb_read_message(union pd_msg *msg) {
  if (!FRToSI2C2.lock2()) {
    asm("bkpt");
  }
  static uint8_t garbage[4];
  uint8_t        numobj;

  // Read the header. If its not a SOP we dont actually want it at all
  // But on some revisions of the fusb if you dont both pick them up and read them out of the fifo, it gets stuck
  fusb_read_byte(FUSB_FIFOS);
  /* Read the message header into msg */
  fusb_read_buf(FUSB_FIFOS, 2, msg->bytes);
  /* Get the number of data objects */
  numobj = PD_NUMOBJ_GET(msg);
  /* If there is at least one data object, read the data objects */
  if (numobj > 0) {
    fusb_read_buf(FUSB_FIFOS, numobj * 4, msg->bytes + 2);
  }
  /* Throw the CRC32 in the garbage, since the PHY already checked it. */
  fusb_read_buf(FUSB_FIFOS, 4, garbage);

  FRToSI2C2.unlock2();
  return 0;
}

void fusb_send_hardrst() {

  if (!FRToSI2C2.lock2()) {
    return;
  }
  /* Send a hard reset */
  fusb_write_byte(FUSB_CONTROL3, 0x07 | FUSB_CONTROL3_SEND_HARD_RESET);

  FRToSI2C2.unlock2();
}

bool fusb_setup() {

  if (!FRToSI2C2.lock2()) {
    return false;
  }

  //fusb_send_hardrst();	//没卵用？？
  /* Fully reset the FUSB302B */
  /**
   * 加完全复位在使用ODGIRON-HUB拔下USB5V供电后PD供电会中止一两秒恢复供电时会自动诱骗到20V
   * 若没有加完全复位，那么上述情况恢复供电时，不会自动诱骗到20V
   */
  fusb_write_byte( FUSB_RESET, FUSB_RESET_SW_RES);
  osDelay(2);


  if (!fusb_read_id()) {
    return false;
  }

  /* Turn on all power */
  fusb_write_byte(FUSB_POWER, 0x0F);

  /* Set interrupt masks */
  // Setting to 0 so interrupts are allowed
  fusb_write_byte(FUSB_MASK1, 0x00);
  fusb_write_byte(FUSB_MASKA, 0x00);
  fusb_write_byte(FUSB_MASKB, 0x00);
  fusb_write_byte(FUSB_CONTROL0, 0b11 << 2);

  /* Enable automatic retransmission */
  fusb_write_byte(FUSB_CONTROL3, 0x07);
  // set defaults
  fusb_write_byte(FUSB_CONTROL2, 0x00);
  /* Flush the RX buffer */
  fusb_write_byte(FUSB_CONTROL1, FUSB_CONTROL1_RX_FLUSH);

  /* Measure CC1 */
  fusb_write_byte(FUSB_SWITCHES0, 0x07);
  osDelay(10);
  uint8_t cc1 = fusb_read_byte(FUSB_STATUS0) & FUSB_STATUS0_BC_LVL;

  /* Measure CC2 */
  fusb_write_byte(FUSB_SWITCHES0, 0x0B);
  osDelay(10);
  uint8_t cc2 = fusb_read_byte(FUSB_STATUS0) & FUSB_STATUS0_BC_LVL;

  /* Select the correct CC line for BMC signaling; also enable AUTO_CRC */
  if (cc1 > cc2) {
    fusb_write_byte(FUSB_SWITCHES1, 0x25);
    fusb_write_byte(FUSB_SWITCHES0, 0x07);
  } else {
    fusb_write_byte(FUSB_SWITCHES1, 0x26);
    fusb_write_byte(FUSB_SWITCHES0, 0x0B);
  }
  FRToSI2C2.unlock2();
  fusb_reset();

  //使用PB1代替PA9
  //TS80P检测FUSB302中断的EXTI GPIO引脚是PA9，但TS100的KEY_A是pin9
  //ODGIRON使用PB1，在MX_GPIO_Init()中完成配置
  /*
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  */
  return true;
}


void fusb_get_status(union fusb_status *status) {
  // xTaskGetSchedulerState() 获取任务调度器的壮态，开启或未开启。
	//若任务调度器是开启的
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    if (!FRToSI2C2.lock2()) {
      return;
    }
  }

  /* Read the interrupt and status flags into status */
  fusb_read_buf(FUSB_STATUS0A, 7, status->bytes);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    FRToSI2C2.unlock2();
  }
}

enum fusb_typec_current fusb_get_typec_current() {
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    if (!FRToSI2C2.lock2()) {
      return fusb_tcc_none;
    }
  }
  /* Read the BC_LVL into a variable */
  enum fusb_typec_current bc_lvl = (enum fusb_typec_current)(fusb_read_byte(FUSB_STATUS0) & FUSB_STATUS0_BC_LVL);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    FRToSI2C2.unlock2();
  }
  return bc_lvl;
}

void fusb_reset() {
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    if (!FRToSI2C2.lock2()) {
      return;
    }
  }
  //ReclaimerLabs/usb-demo/tcpm.h
  //this->tcpc_write(TCPC_REG_RESET, TCPC_REG_RESET_PD_RESET);	//8bit i2c写val = (1 << 1)到复位寄存器0X0C
  /* 复位FUSB302B，原本没有，我加上了，不然自复位后pd状态还是不变
   * fusb_reset()在 ProtocolTransmit::thread() switch case PRLTxPHYReset 的 protocol_tx_phy_reset()中调用*/
  //fusb_write_byte(FUSB_RESET,  FUSB_RESET_PD_RESET);
  //fusb_write_byte(FUSB_RESET,  FUSB_RESET_SW_RES);	// 软复位所有寄存器 Reset the FUSB302B including the I2C registers to their default values
  /* Flush the TX buffer */
  fusb_write_byte(FUSB_CONTROL0, 0x44);
  /* Flush the RX buffer */
  fusb_write_byte(FUSB_CONTROL1, FUSB_CONTROL1_RX_FLUSH);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    FRToSI2C2.unlock2();
  }
}

/**
 * Read Device ID
 */
bool fusb_read_id() {
  // Return true if read of the revision ID is sane
  uint8_t version = 0;
  fusb_read_buf(FUSB_DEVICE_ID, 1, &version);
  if (version == 0 || version == 0xFF)
    return false;
  return true;
}


uint8_t fusb302_detect() {
  // Probe the I2C bus for its address
  return FRToSI2C2.probe(FUSB302B_ADDR);
}

#endif
