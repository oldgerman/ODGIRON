/*
  This is a library written for the STUSB4500 Power Delivery Board.
  SparkFun sells these at its website: https://www.sparkfun.com
  
  The functions were based on the NVM_Flasher code for
  the STUSB4500 which can be found here: https://github.com/usb-c/STUSB4500

  Written by Alex Wende @ SparkFun Electronics, February 6th, 2020

  https://github.com/sparkfun/SparkFun_STUSB4500_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

  Development environment specifics:
  Arduino IDE 1.8.6

  For licence information see LICENSE.md
  https://github.com/sparkfun/SparkFun_STUSB4500_Arduino_Library/blob/master/LICENSE.md
*/

#include "SparkFun_STUSB4500.h"

uint8_t sector[5][8];
uint8_t readSectors = 0;

uint8_t STUSB4500::begin(uint8_t deviceAddress, TwoWire &wirePort)
{
  readSectors = 0;
  _deviceAddress = deviceAddress; //If provided, store the I2C address from user
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  _i2cPort->beginTransmission(_deviceAddress);

  uint8_t error = _i2cPort->endTransmission();

  if(error == 0)
  {
	if(readSectors == 0)
    {
      read();
	  readSectors = 1;
    }
	return true; //Device online!
  }
  else return false;          //Device not attached?
}

void STUSB4500::read(void)
{
  uint8_t Buffer[2];
  readSectors = 1;
  //Read Current Parameters
  //-Enter Read Mode
  //-Read Sector[x][-]
  //---------------------------------
  //Enter Read Mode
  Buffer[0]=FTP_CUST_PASSWORD;  /* Set Password 0x95->0x47*/
  I2C_Write_USB_PD(FTP_CUST_PASSWORD_REG,Buffer,1);

  Buffer[0]= 0; /* NVM internal controller reset 0x96->0x00*/
  I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1);
  
  Buffer[0]= FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits 0x96->0xC0*/
  I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1);

  //--- End of CUST_EnterReadMode

  for(uint8_t i=0;i<5;i++)
  {
    Buffer[0]= FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits 0x96->0xC0*/
    I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1);

    Buffer[0]= (READ & FTP_CUST_OPCODE);  /* Set Read Sectors Opcode 0x97->0x00*/
    I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1);

    Buffer[0]= (i & FTP_CUST_SECT) |FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;
    I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1);  /* Load Read Sectors Opcode */

    do 
    {
      I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1); /* Wait for execution */
    }
    while(Buffer[0] & FTP_CUST_REQ); //The FTP_CUST_REQ is cleared by NVM controller when the operation is finished.

    I2C_Read_USB_PD(RW_BUFFER,&sector[i][0],8);
  }
  
  CUST_ExitTestMode();
}

void STUSB4500::write(uint8_t defaultVals)
{
  if(defaultVals == 0)
  {
	CUST_EnterWriteMode(SECTOR_0 | SECTOR_1  | SECTOR_2 | SECTOR_3  | SECTOR_4 );
    CUST_WriteSector(0,&sector[0][0]);
    CUST_WriteSector(1,&sector[1][0]);
    CUST_WriteSector(2,&sector[2][0]);
    CUST_WriteSector(3,&sector[3][0]);
    CUST_WriteSector(4,&sector[4][0]);
    CUST_ExitTestMode();
  }
  else
  {
	uint8_t default_sector[5][8] = 
    {
      {0x00,0x00,0xB0,0xAA,0x00,0x45,0x00,0x00},
      {0x10,0x40,0x9C,0x1C,0xFF,0x01,0x3C,0xDF},
      {0x02,0x40,0x0F,0x00,0x32,0x00,0xFC,0xF1},
      {0x00,0x19,0x56,0xAF,0xF5,0x35,0x5F,0x00},
      {0x00,0x4B,0x90,0x21,0x43,0x00,0x40,0xFB}
    };
  
	CUST_EnterWriteMode(SECTOR_0 | SECTOR_1  | SECTOR_2 | SECTOR_3  | SECTOR_4 );
    CUST_WriteSector(0,&default_sector[0][0]);
    CUST_WriteSector(1,&default_sector[1][0]);
    CUST_WriteSector(2,&default_sector[2][0]);
    CUST_WriteSector(3,&default_sector[3][0]);
    CUST_WriteSector(4,&default_sector[4][0]);
    CUST_ExitTestMode();
  }
  
}

float STUSB4500::getVoltage(uint8_t pdo_numb)
{  
  if(pdo_numb == 1) //PDO1
  {
	return 5;
  }
  else if(pdo_numb == 2) //PDO2
  {
	return sector[4][1]*0.2;
  }
  else //PDO3
  {
	return (((sector[4][3]&0x03)<<8) + sector[4][2])*0.05;
  }
}

float STUSB4500::getCurrent(uint8_t pdo_numb)
{
  uint8_t digitalValue;
  
  if(pdo_numb == 1) //PDO1
  {
	digitalValue = (sector[3][2]&0xF0) >> 4;
	
    if(digitalValue == 0)      return 0;
    else if(digitalValue < 11) return digitalValue * 0.25 + 0.25;
    else                       return digitalValue * 0.50 - 2.50;
  }
  else if(pdo_numb == 2) //PDO2
  {
	digitalValue = (sector[3][4]&0x0F);
	
    if(digitalValue== 0)       return 0;
    else if(digitalValue < 11) return digitalValue * 0.25 + 0.25;
    else                       return digitalValue * 0.50 - 2.50;
  }
  else //PDO3
  {
	digitalValue = (sector[3][5]&0xF0) >> 4;
	
    if(digitalValue == 0)      return 0;
    else if(digitalValue < 11) return digitalValue * 0.25 + 0.25;
    else                       return digitalValue * 0.50 - 2.50;
  }
}

uint8_t STUSB4500::getLowerVoltageLimit(uint8_t pdo_numb)
{  
  if(pdo_numb == 1) //PDO1
  {
	return 0;
  }
  else if(pdo_numb == 2) //PDO2
  {
	return (sector[3][4]>>4) + 5;
  }
  else //PDO3
  {
	return (sector[3][6] & 0x0F) + 5;
  }
}

uint8_t STUSB4500::getUpperVoltageLimit(uint8_t pdo_numb)
{
  if(pdo_numb == 1) //PDO1
  {
	return (sector[3][3]>>4) + 5;
  }
  else if(pdo_numb == 2) //PDO2
  {
	return (sector[3][5] & 0x0F) + 5;
  }
  else //PDO3
  {
	return (sector[3][6]>>4) + 5;
  }
}

float STUSB4500::getFlexCurrent(void)
{
  uint16_t digitalValue = ((sector[4][4]&0x0F)<<6) + ((sector[4][3]&0xFC)>>2);
  return digitalValue / 100.0;
}

uint8_t STUSB4500::getPdoNumber(void)
{
  return (sector[3][2] & 0x06)>>1;
}

uint8_t STUSB4500::getExternalPower(void)
{
  return (sector[3][2]&0x08)>>3;
}

uint8_t STUSB4500::getUsbCommCapable(void)
{
  return (sector[3][2]&0x01);
}

uint8_t STUSB4500::getConfigOkGpio(void)
{
  return (sector[4][4]&0x60)>>5;
}

uint8_t STUSB4500::getGpioCtrl(void)
{
  return (sector[1][0]&0x30)>>4;
}

uint8_t STUSB4500::getPowerAbove5vOnly(void)
{
  return (sector[4][6]&0x08)>>3;
}

uint8_t STUSB4500::getReqSrcCurrent(void)
{
  return (sector[4][6]&0x10)>>4;
}

void STUSB4500::setVoltage(uint8_t pdo_numb, float voltage)
{
  //Constrain voltage variable to 5-20V
  if(voltage < 5) voltage = 5;
  else if(voltage > 20) voltage = 20;
  
  //PDO1 Fixed at 5V
  
  if(pdo_numb == 2) //PDO2
  {
	sector[4][1] = voltage/0.2; //load Voltage (sector 4, byte 1, bits 0:7)
  }
  else //PDO3
  {
	// Load voltage (10-bit)
    // -bit 8:9 - sector 4, byte 3, bits 0:1
    // -bit 0:7 - sector 4, byte 2, bits 0:7
    uint16_t setVoltage = voltage/0.05;   //convert voltage to 10-bit value
    sector[4][2] = 0xFF & setVoltage;   //load bits 0:7
    sector[4][3] &= 0xFC;               //clear bits 0:1
    sector[4][3] |= (setVoltage>>8);    //load bits 8:9
  }
}

void STUSB4500::setCurrent(uint8_t pdo_numb, float current)
{
  /*Convert current from float to 4-bit value
    -current from 0.5-3.0A is set in 0.25A steps
    -current from 3.0-5.0A is set in 0.50A steps
  */
  if(current < 0.5)     current = 0;
  else if(current <= 3) current = (4*current)-1;
  else                  current = (2*current)+5;
  
  if(pdo_numb == 1) //PDO1
  {
	//load current (sector 3, byte 2, bits 4:7)
    sector[3][2] &= 0x0F;             //clear bits 4:7
    sector[3][2] |= ((int)current<<4);    //load new amperage for PDO1
  }
  else if(pdo_numb == 2) //PDO2
  {
	//load current (sector 3, byte 4, bits 0:3)
    sector[3][4] &= 0xF0;             //clear bits 0:3
    sector[3][4] |= (int)current;     //load new amperage for PDO2
  }
  else //PDO3
  {
	//load current (sector 3, byte 5, bits 4:7)
    sector[3][5] &= 0x0F;           //clear bits 4:7
    sector[3][5] |= ((int)current<<4);  //set amperage for PDO3
  }  
}

void STUSB4500::setLowerVoltageLimit(uint8_t pdo_numb, uint8_t value)
{
  //Constrain value to 5-20%
  if(value < 5) value = 5;
  else if(value > 20) value = 20;
  
  //UVLO1 fixed

  if(pdo_numb == 2) //UVLO2
  {
    //load UVLO (sector 3, byte 4, bits 4:7)
    sector[3][4] &= 0x0F;             //clear bits 4:7
    sector[3][4] |= (value-5)<<4;  //load new UVLO value
  }
  else if(pdo_numb == 3) //UVLO3
  {
    //load UVLO (sector 3, byte 6, bits 0:3)
    sector[3][6] &= 0xF0;
    sector[3][6] |= (value-5);
  }
}

void STUSB4500::setUpperVoltageLimit(uint8_t pdo_numb, uint8_t value)
{
  //Constrain value to 5-20%
  if(value < 5) value = 5;
  else if(value > 20) value = 20;

  if(pdo_numb == 1) //OVLO1
  {
    //load OVLO (sector 3, byte 3, bits 4:7)
    sector[3][3] &= 0x0F;             //clear bits 4:7
    sector[3][3] |= (value-5)<<4;  //load new OVLO value
  }
  else if(pdo_numb == 2) //OVLO2
  {
    //load OVLO (sector 3, byte 5, bits 0:3)
    sector[3][5] &= 0xF0;             //clear bits 0:3
    sector[3][5] |= (value-5);     //load new OVLO value
  }
  else if(pdo_numb == 3) //OVLO3
  {
    //load OVLO (sector 3, byte 6, bits 4:7)
    sector[3][6] &= 0x0F;
    sector[3][6] |= ((value-5)<<4);
  }
}

void STUSB4500::setFlexCurrent(float value)
{
  //Constrain value to 0-5A
  if(value > 5) value = 5;
  else if(value < 0) value = 0;
  
  uint16_t flex_val = value*100;

  sector[4][3] &= 0x03;                 //clear bits 2:6
  sector[4][3] |= ((flex_val&0x3F)<<2); //set bits 2:6
  
  sector[4][4] &= 0xF0;                 //clear bits 0:3
  sector[4][4] |= ((flex_val&0x3C0)>>6);//set bits 0:3
}

void STUSB4500::setPdoNumber(uint8_t value)
{
  if(value > 3) value = 3;
  
  //load PDO number (sector 3, byte 2, bits 2:3)
  sector[3][2] &= 0xF9;
  sector[3][2] |= (value<<1);
}

void STUSB4500::setExternalPower(uint8_t value)
{
  if(value != 0) value = 1;
  
  //load SNK_UNCONS_POWER (sector 3, byte 2, bit 3)
  sector[3][2] &= 0xF7; //clear bit 3
  sector[3][2] |= (value)<<3;
}

void STUSB4500::setUsbCommCapable(uint8_t value)
{
  if(value != 0) value = 1;
  
  //load USB_COMM_CAPABLE (sector 3, byte 2, bit 0)
  sector[3][2] &= 0xFE; //clear bit 0
  sector[3][2] |= (value);
}

void STUSB4500::setConfigOkGpio(uint8_t value)
{
  if(value < 2) value = 0;
  else if(value > 3) value = 3;
  
  //load POWER_OK_CFG (sector 4, byte 4, bits 5:6)
  sector[4][4] &= 0x9F; //clear bit 3
  sector[4][4] |= value<<5;
}

void STUSB4500::setGpioCtrl(uint8_t value)
{
  if(value > 3) value = 3;
  
  //load GPIO_CFG (sector 1, byte 0, bits 4:5)
  sector[1][0] &= 0xCF; //clear bits 4:5
  sector[1][0] |= value<<4;
}

void STUSB4500::setPowerAbove5vOnly(uint8_t value)
{
  if(value != 0) value = 1;
  
  //load POWER_ONLY_ABOVE_5V (sector 4, byte 6, bit 3)
  sector[4][6] &= 0xF7; //clear bit 3
  sector[4][6] |= (value<<3); //set bit 3
}

void STUSB4500::setReqSrcCurrent(uint8_t value)
{
  if(value != 0) value = 1;
  
  //load REQ_SRC_CURRENT (sector 4, byte 6, bit 4)
  sector[4][6] &= 0xEF; //clear bit 4
  sector[4][6] |= (value<<4); //set bit 4
}

uint8_t STUSB4500::CUST_EnterWriteMode(unsigned char ErasedSector)
{
  unsigned char Buffer[2];
  
  
  Buffer[0]=FTP_CUST_PASSWORD;   /* Set Password*/
  if ( I2C_Write_USB_PD(FTP_CUST_PASSWORD_REG,Buffer,1) != 0 )return -1;
  
  Buffer[0]= 0 ;   /* this register must be NULL for Partial Erase feature */
  if ( I2C_Write_USB_PD(RW_BUFFER,Buffer,1) != 0 )return -1;
  
  {
    //NVM Power-up Sequence
    //After STUSB start-up sequence, the NVM is powered off.
    
    Buffer[0]= 0;  /* NVM internal controller reset */
    if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1)  != 0 ) return -1;
    
    Buffer[0]= FTP_CUST_PWR | FTP_CUST_RST_N; /* Set PWR and RST_N bits */
    if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1) != 0 ) return -1;
  }
  
  
  Buffer[0]=((ErasedSector << 3) & FTP_CUST_SER) | ( WRITE_SER & FTP_CUST_OPCODE) ;  /* Load 0xF1 to erase all sectors of FTP and Write SER Opcode */
  if ( I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1) != 0 )return -1; /* Set Write SER Opcode */
  
  Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ; 
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1)  != 0 )return -1; /* Load Write SER Opcode */
  
  do 
  {
      delay(500);
      if ( I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Wait for execution */
  }
  while(Buffer[0] & FTP_CUST_REQ); 
  
  Buffer[0]=  SOFT_PROG_SECTOR & FTP_CUST_OPCODE ;  
  if ( I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1) != 0 )return -1;  /* Set Soft Prog Opcode */
  
  Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ; 
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1)  != 0 )return -1; /* Load Soft Prog Opcode */
    
  do 
  {
    if ( I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Wait for execution */
  }
  while(Buffer[0] & FTP_CUST_REQ);
  
  Buffer[0]= ERASE_SECTOR & FTP_CUST_OPCODE ;  
  if ( I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1) != 0 )return -1; /* Set Erase Sectors Opcode */
  
  Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N | FTP_CUST_REQ ;  
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1)  != 0 )return -1; /* Load Erase Sectors Opcode */
  
  do 
  {
    if ( I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Wait for execution */
  }
  while(Buffer[0] & FTP_CUST_REQ);  
    
  return 0;
}

uint8_t STUSB4500::CUST_ExitTestMode(void)
{
  unsigned char Buffer[2];
  
  Buffer[0]= FTP_CUST_RST_N;
  Buffer[1]= 0x00;  /* clear registers */
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1;
  
  Buffer[0]= 0x00;
  if ( I2C_Write_USB_PD(FTP_CUST_PASSWORD_REG,Buffer,1) != 0 )return -1;  /* Clear Password */
  
  return 0 ;
}

uint8_t STUSB4500::CUST_WriteSector(char SectorNum, unsigned char *SectorData)
{
  unsigned char Buffer[2];
  
  //Write the 64-bit data to be written in the sector
  if ( I2C_Write_USB_PD(RW_BUFFER,SectorData,8) != 0 )return -1;
  
  Buffer[0]=FTP_CUST_PWR | FTP_CUST_RST_N; /*Set PWR and RST_N bits*/
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1;
  
  //NVM Program Load Register to write with the 64-bit data to be written in sector
  Buffer[0]= (WRITE_PL & FTP_CUST_OPCODE); /*Set Write to PL Opcode*/
  if ( I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1) != 0 )return -1;
  
  Buffer[0]=FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;  /* Load Write to PL Sectors Opcode */  
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1;
  
  do 
  {
    if ( I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Wait for execution */
  }     
  while(Buffer[0] & FTP_CUST_REQ) ; //FTP_CUST_REQ clear by NVM controller
  
  
  //NVM "Word Program" operation to write the Program Load Register in the sector to be written
  Buffer[0]= (PROG_SECTOR & FTP_CUST_OPCODE);
  if ( I2C_Write_USB_PD(FTP_CTRL_1,Buffer,1) != 0 )return -1;/*Set Prog Sectors Opcode*/
  
  Buffer[0]=(SectorNum & FTP_CUST_SECT) |FTP_CUST_PWR |FTP_CUST_RST_N | FTP_CUST_REQ;
  if ( I2C_Write_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Load Prog Sectors Opcode */  
  
  do 
  {
    if ( I2C_Read_USB_PD(FTP_CTRL_0,Buffer,1) != 0 )return -1; /* Wait for execution */
  }
  while(Buffer[0] & FTP_CUST_REQ); //FTP_CUST_REQ clear by NVM controller
  
  return 0;
}

uint8_t STUSB4500::I2C_Write_USB_PD(uint16_t Register ,uint8_t *DataW ,uint16_t Length)
{
  uint8_t error;
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(Register);
  for(uint8_t i=0;i<Length;i++)
  {
    _i2cPort->write(*(DataW+i));
  }
  error = _i2cPort->endTransmission();
  delay(1);

  return error;  
}

uint8_t STUSB4500::I2C_Read_USB_PD(uint16_t Register ,uint8_t *DataR ,uint16_t Length)
{   
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(Register);
  _i2cPort->endTransmission();
  _i2cPort->requestFrom(_deviceAddress,Length);
  uint8_t tempData[Length];
  for(uint16_t i=0;i<Length;i++)
  {
    tempData[i] = _i2cPort->read();
  }
  memcpy(DataR,tempData,Length);
  
  return 0;
}
