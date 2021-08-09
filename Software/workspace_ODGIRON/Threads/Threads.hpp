/*
 * Threads.h
 *
 *  Created on: Feb 16, 2021
 *      Author: OldGerman
 */

#ifndef THREADS_THREADS_HPP_
#define THREADS_THREADS_HPP_

#include "BSP.h"
#include "FreeRTOS.h"
#include "Settings.h"
#include "TipThermoModel.h"
#include "cmsis_os.h"
#include <history.hpp>
#include "main.h"
#include "power.hpp"
#include "task.h"
#include <Buttons.hpp>
#include "oled_init.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif
#include "Colum.hpp"
#include "Arduino.h"
#ifdef __cplusplus
void GUIDelay();
extern AutoValue screenBrightness;
extern AutoValue solderingTemp;
extern "C" {
#endif
extern ButtonState buttons;
enum TipState {
	TIP_SLEEPING,
	TIP_HEATING,
	TIP_OPEN_CIRCUIT,
	TIP_SHUT_DOWN
};

extern enum TipState tipState;

#define NO_DETECTED_ACCELEROMETER 99
#define ACCELEROMETERS_SCANNING   100
extern uint8_t DetectedAccelerometerVersion;
extern TaskHandle_t      pidTaskNotification;
extern uint32_t          currentTempTargetDegC;
extern TickType_t lastMovementTime;
extern char  busVChar[8], shuntChar[10], busMAChar[10], busMWChar[10];  // 储存INA226读出数据的buffers
extern  uint16_t busV;
extern  uint16_t busAX1000;
extern bool buttonChanged;
extern bool buttonIsBeep;
extern uint8_t  INADevicesFound;
extern uint16_t tipTempMaxAdjust;		   //焊接模式可调温度上限
extern uint16_t tipDisconnectedThres;
extern bool moveDetected;
/* setup */
void doGUITask();
void doMOVTask();
void doPOWTask();
void doPIDTask();
void doUSBTask();

void setContrast(uint16_t);
uint16_t getRefTemperatureX10();
void shutScreen();
void brightScreen();
extern uint8_t buffTest[];

#ifdef __cplusplus
extern void drawNumber(uint8_t x, uint8_t y, uint16_t number, uint8_t places, uint8_t padixPointOffset = 0);
}
#endif


#endif /* THREADS_THREADS_HPP_ */
