/*
 * Settings.h
 *
 *  Created on: 29 Sep 2016
 *      Author: Ralim
 *		Modify: OldGerman
 *      Houses the system settings and allows saving / restoring from flash
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_
#include "main.h"
#ifndef ODGIRON_BOOTLDR
#include "Settings.h"
#endif
#ifdef __cplusplus
extern "C" {

#include <stdint.h>

extern uint16_t settings_page[512];
#define SETTINGSVERSION (0x24)
/*Change this if you change the struct below to prevent people getting \
          out of sync*/

/*
 * 用于储存ironOS所有的设置信息
 * 此结构必须是2bytes(16bit)的倍数，因为它是以uint16_t块的形式在flash里保存/加载
 * This struct must be a multiple of 2 bytes as it is saved / restored from
 * flash in uint16_t chunks
 */
typedef struct {
  uint8_t 	version;			// Used to track if a reset is needed on firmware upgrade
  // 焊接设置
  uint16_t  AutoStartMode;   	// 通电后进是否应自动直接入焊接模式
  uint16_t  MenuKeepHeating;		// 菜单页保持加热
  uint16_t 	SolderingTemp;      // current set point for the iron
  uint16_t 	SleepTemp;          // temp to drop to in sleep
  uint16_t  ShutDownTemp;
  uint16_t 	TempOffsetRef;		// 基准温度偏移量（取自模拟温度传感器）
  // 休眠设置
  uint16_t	SleepMode;
  uint16_t  SleepTime;           // Sec timeout to sleep
  uint16_t  ShutDownTime;        // Ses timeout from sleep to shut down
  uint16_t  ShutDownMode;
  // 屏幕设置
  uint16_t ScreenBrightness;	// 0~100% 屏幕亮度
  uint16_t  OrientationMode; 	// Selects between Auto,Right and left handed layouts		//0左手 1右手 2自动旋转
  //其他参数设置
  uint16_t  Sensitivity;     	// 0~100% 加速度计灵敏度
  uint16_t BuzzerVolume;		// 0~100% 蜂鸣器音量
  //辅助功能
  uint16_t  ResetForceDFU;		//重启跳过logo页面直接进入DFU标记
  uint16_t CalibrationOffset; // This stores the temperature offset for this tip
  // in the iron.
  uint16_t  Language;
#ifdef ENABLED_FAHRENHEIT_SUPPORT
  uint8_t temperatureInF : 1; // Should the temp be in F or C (true is F)
#endif
  uint16_t descriptionScrollSpeed; 		// Description scroll speed

  uint16_t powerLimit; 					// 功率限制 Maximum power iron allowed to output
  uint16_t  KeepAwakePulse;             	// 保持唤醒功率 Keep Awake pulse power in 0.1 watts (10 = 1Watt)
  uint16_t BoostTemp;         			// Boost mode set point for the iron

  uint8_t  ReverseButtonTempChangeEnabled; // Change the plus and minus button assigment
  uint16_t TempChangeLongStep;             // Change the plus and minus button assigment
  uint16_t TempChangeShortStep;            // Change the plus and minus button assigment

  uint8_t detailedSoldering : 1; // Detailed soldering screens

} systemSettingsType;

extern systemSettingsType systemSettings;

void     saveSettings();
bool     restoreSettings();
uint8_t  lookupVoltageLevel();
uint16_t lookupHallEffectThreshold();
void     resetSettings();
#endif
#ifdef __cplusplus
}
#endif



#endif /* SETTINGS_H_ */
