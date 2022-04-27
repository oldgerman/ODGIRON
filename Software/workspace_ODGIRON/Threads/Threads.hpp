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
#include "cmsis_os.h"
#include <history.hpp>
#include "main.h"
#include "power.hpp"
#include "task.h"
#include <Buttons.hpp>
#include <TipThermoModel.hpp>
#include "oled_init.h"
#ifndef STM32F1
#include "usbd_cdc_if.h"
#endif
#include "Colum.hpp"
#include "Arduino.h"
#include "LIS3DH.hpp"
#ifdef __cplusplus
void GUIDelay();
extern AutoValue screenBrightness;
extern AutoValue solderingTemp;
extern AxisData axisData;
extern AxisAvg axAvg;
extern bool BeepDouble;
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

//卡尔曼滤波
typedef struct
{
    float LastP;//上次估算协方差 初始化值为0.02
    float Now_P;//当前估算协方差 初始化值为0
    float out;//卡尔曼滤波器输出 初始化值为0
    float Kg;//卡尔曼增益 初始化值为0
    uint16_t *Q;//过程噪声协方差 初始化值为0.001
    uint16_t *R;//观测噪声协方差 初始化值为0.543
} KFP;//Kalman Filter parameter
float kalmanFilter(KFP* kfp, float input);
extern KFP KFP_Temp;
#ifdef __cplusplus
extern void drawNumber(uint8_t x, uint8_t y, uint16_t number, uint8_t places, uint8_t padixPointOffset = 0);
extern int32_t getX10WattageLimits();
extern bool heaterThermalRunaway;
}
#endif


#endif /* THREADS_THREADS_HPP_ */
