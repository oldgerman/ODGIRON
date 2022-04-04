/*
 * GUIThread.cpp
 *
 *  Created on: 19 Aug 2019
 *      Author: ralim
 *      Modify: OldGerman
 */

#include "Threads.hpp"
#include "I2C_Wrapper.h"

#include "INA.h"  // Zanshin INA Library
#include "dtostrf.h"  // Needed for the SAM3XA (Arduino Zero)


/**************************************************************************************************
** Declare program constants, global variables and instantiate INA class                         **
**************************************************************************************************/
//const uint32_t SERIAL_SPEED{115200};     ///< Use fast serial speed
//请根据实际偏差调小采样电阻的值
const uint32_t SHUNT_MICRO_OHM{10000};   //ODGIRON的分流电阻器为0.01mΩ对应为10000///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS{50};          //最大期望测量电流，实测T12烙铁头20V满载2.5A///< Max expected amps, clamped from 1A to a max of 1022A
uint8_t        INADevicesFound{0};          ///< Number of INAs found
//INA_Class      INA;                    ///INA类实例以使用EEPROM< INA class instantiation to use EEPROM
// INA_Class      INA(0);                ///INA类实例以使用EEPROM< INA class instantiation to use EEPROM
 INA_Class      INA(1);                  ///INA类实例使用动态内存，而不是EEPROM。最多为（n）个设备分配存储空间< INA class instantiation to use dynamic memory rather
// 					^ n个设备			 ///   than EEPROM. Allocate storage for up to (n) devices
char     busVChar[8], shuntChar[10], busMAChar[10], busMWChar[10];  // 储存INA226读出数据的buffers
uint16_t busV = 0;
uint16_t busAX1000 = 0;
void doPOWTask()
{
	postRToSInit();	//检测FUSB302 I2C应答，并创建4个线程
	//uint16_t loopCounter = 0;     // Count the number of iterations
	char     sprintfBuffer[200];  // Buffer to format output
	  /************************************************************************************************
	  ** The INA.begin call initializes the device(s) found with an expected ±1 Amps maximum current **
	  ** and for a 0.1Ohm resistor, and since no specific device is given as the 3rd parameter all   **
	  ** devices are initially set to these values.                                                  **
	  ************************************************************************************************/
	INADevicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
	  //while (devicesFound == 0) {
	  //  usb_printf("No INA device found, retrying in 1 seconds...\r\n");
	  //  delay(1000);                                             // Wait 10 seconds before retrying
	  //  devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
	  //}                                                           // while no devices detected
	  //usb_printf(" - Detected %d INA devices on the I2C bus\r\n", devicesFound);

	  INA.setBusConversion(8500);             // Maximum conversion time 8.244ms
	  INA.setShuntConversion(8500);           // Maximum conversion time 8.244ms
	  INA.setAveraging(128);                  // Average each reading n-times
	  INA.setMode(INA_MODE_CONTINUOUS_BOTH);  // Bus/shunt measured continuously
	  INA.alertOnBusOverVoltage(true, 5000);  // Trigger alert if over 5V on bus

	for(;;)
	{
	    osDelay(TICKS_100MS); // Slow down update rate //在前延迟和在后延迟区别？
	    power_check();

	    static int16_t cnt10 = 0;
	    //cnt10 = (cnt10 + 1) % 5;



		if (cnt10 == 0 &&  tipState != TIP_SHUT_DOWN) {//INA_Class::getDeviceCount() &&
			//usb_printf("Nr Adr Type   Bus      Shunt       Bus         Bus\r\n");
			//usb_printf("== === ====== ======== =========== =========== ===========\r\n");
			for (uint8_t i = 0; i < INADevicesFound; i++) // Loop through all devices
				{
				busV = INA.getBusMilliVolts(i);
				busAX1000 = INA.getBusMicroAmps(i) / 10000;
#if 0
				dtostrf(busV / 1000.0, 6, 3, busVChar); // Convert floating point to char
				dtostrf(INA.getShuntMicroVolts(i) / 1000.0, 6, 3, shuntChar); // Convert floating point to char
				dtostrf(INA.getBusMicroAmps(i) / 10000.0, 7, 3, busMAChar); // Convert floating point to char
				dtostrf(INA.getBusMicroWatts(i) / 10000.0, 8, 3, busMWChar); // Convert floating point to char

				sprintf(sprintfBuffer, "%3d %s %sV %smV %smA %smW",
						INA.getDeviceAddress(i), INA.getDeviceName(i), busVChar,
						shuntChar, busMAChar, busMWChar);
				usb_printf("%s\r\n", sprintfBuffer);
#endif
			    sprintf(sprintfBuffer, "%3d %s",
			    						INA.getDeviceAddress(i), INA.getDeviceName(i));
				//usb_printf("%s\r\n", sprintfBuffer);
				/*
				 68 INA226 20.965V  0.250mV 318.9180mA 6714.0700mW
				 68 INA226 20.962V  0.217mV 277.7180mA 5836.6630mW
				 68 INA226 20.656V 12.075mV 15399.6350mA 315446.8450mW
				 68 INA226 20.550V 16.272mV 20752.5800mA 425237.1490mW
				 *
				 */
			}
		}

#if 0
#ifdef STM32F1
		I2C_HandleTypeDef* hi2cPtr = &hi2c2;
#elif defined(STM32F4)
		I2C_HandleTypeDef* hi2cPtr = &hi2c3;
#else
#endif
		uint8_t i=0;
		HAL_StatusTypeDef status;
		char buffUSB[30] = {0};
		for(i=0; i<127; i++)
		{
			 status = HAL_I2C_Master_Transmit(hi2cPtr,i<<1,0,0,200);

			  if(status==HAL_OK)
			  {
					  sprintf(buffUSB, "i2c addr:0x%02X is ok\r\n",i);
					  usb_printf("%s\r\n",buffUSB);
			  }
			  else if(status==HAL_TIMEOUT)
			  {
				  sprintf(buffUSB, "i2c addr:0x%02X is timeout\r\n",i);
				  usb_printf("%s\r\n",buffUSB);
			  }
			  else if(status==HAL_BUSY)
			  {
				  sprintf(buffUSB, "i2c addr:0x%02X is busy\r\n",i);
				  usb_printf("%s\r\n",buffUSB);
			  }
			  else if(status == HAL_ERROR)
			  {
				  //sprintf(buffUSB, "i2c addr:0x%02X is error\r\n",i);
				  //usb_printf("%s\r\n",buffUSB);
			  }
			  else {
				  usb_printf("status = %d\r\n", status);
			}
			  osDelay(10);
		}
		usb_printf("scan end\r\n");
		osDelay(1000);
		memset(buffUSB,0,30);
#endif
	}
}


