/*
 * FRToSI2C.hpp
 *
 *  Created on: 14Apr.,2018
 *      Author: Ralim <--- IronOS创始人
 */

#ifndef FRTOSI2C_HPP_
#define FRTOSI2C_HPP_


#include "main.h"
#ifndef ODGIRON_BOOTLDR
#include "cmsis_os.h"
#endif
/*
 * Wrapper class to work with the device I2C bus
 *
 * This provides mutex protection of the peripheral
 * Also allows hardware to use DMA should it want to
 *
 *
 * 包装器类可与设备I2C总线一起使用
 *
 * 这为外围设备提供互斥保护：
 * 		（oled和LIS3DH共用I2C1）
 * 还允许硬件在需要时使用DMA
 */
#ifdef __cplusplus

class FRToSI2C {
public:
  FRToSI2C(I2C_HandleTypeDef *I2C_Handle)
	 : _I2C_Handle(I2C_Handle)	{
	  this->_I2CSemaphore = nullptr;
  }
  ~FRToSI2C(){}

  /* 创建I2C1的静态二值信号量(static模式)，并释放I2CSemaphore */
  void FRToSInit();

  static void CpltCallback(I2C_HandleTypeDef *); // Normal Tx Callback

  bool    Mem_Read(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size);
  bool 	  Mem_Write(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size);
  bool    probe(uint16_t DevAddress);	// Returns true if device ACK's being addressed
  bool    wakePart(uint16_t DevAddress);	//未实现
  bool    Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
  void    Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
  bool    Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
  bool    Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
  void    TransmitReceive(uint16_t DevAddress, uint8_t *pData_tx, uint16_t Size_tx,
		  	  	  	  	  uint8_t *pData_rx, uint16_t Size_rx);
  bool    I2C_RegisterWrite(uint8_t address, uint8_t reg, uint8_t data);
  uint8_t I2C_RegisterRead(uint8_t address, uint8_t reg);

  /*
   * 唯一的非static
   * I2C_REG是oled和LIS3DH各自唯一私有的数据成员，其他操作都是共用static函数
   *
   * 这个数据成员是使用非常有意思，在oled和LIS2DH12中都是在其.cpp中创建I2C_REG数组
   */
  typedef struct {
    const uint8_t reg;      // 需要写入的寄存器地址			//The register to write to
    uint8_t       val;      // 需要像该寄存器写入到值		//The value to write to this register
    const uint8_t pause_ms; // 编写此寄存器后要暂停多少毫秒 // How many ms to pause _after_ writing this reg
  } I2C_REG;

  bool writeRegistersBulk(const uint8_t address, const I2C_REG *registers, const uint8_t registersLength);
  I2C_HandleTypeDef * getI2C_Handle() { return _I2C_Handle; }
  SemaphoreHandle_t * getI2CSemaphore() { return &_I2CSemaphore; }
  void unlock2();
  bool lock2();

private:
  void              	unlock();
  bool              	lock();
  void              	I2C_Unstick();
  SemaphoreHandle_t 	_I2CSemaphore2;	//信号量2用于软件的
  StaticSemaphore_t 	_xSemaphoreBuffer2;
  SemaphoreHandle_t 	_I2CSemaphore;			//I2Cx信号量
  StaticSemaphore_t 	_xSemaphoreBuffer;  //用来保存信号量结构体，为啥不用指针？
  I2C_HandleTypeDef *   _I2C_Handle;		//指向HAL库I2C句柄

};

extern FRToSI2C FRToSI2C1;
extern FRToSI2C FRToSI2C2;

extern "C" {

extern uint8_t oledPage;	//用于标记page号, 每次在I2C1 DMA传输完成回调函数中自增1，计数到3再归0
#endif

#ifdef __cplusplus
}
#endif
#endif /* FRTOSI2C_HPP_ */
