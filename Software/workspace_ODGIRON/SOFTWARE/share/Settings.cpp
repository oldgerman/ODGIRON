/*
 * Settings.c
 *
 *  Created on: 29 Sep 2016
 *      Author: Ralim
 *		Modify: OldGerman
 *      This file holds the users settings and saves / restores them to the
 * devices flash
 */

#include "Settings.h"
#include "configuration.h"
#include "flash.h"
#include "string.h"
#ifndef ODGIRON_BOOTLDR
#include "BSP.h"
#include "Setup.h"
#include  "cmsis_os.h"
#else
#include "cppports.h"
#endif


/*这个变量是未初始化的，只分配了地址，由程序运行时擦写它*/
uint16_t settings_page[512] __attribute__((section(".settings_page")));

systemSettingsType systemSettings;		//用于在内存储存ironOS所有的设置信息

void saveSettings() {
  // First we erase the flash
  flash_save_buffer((uint8_t *)&systemSettings, sizeof(systemSettingsType));
}

/*将Flash settings_page区的值读给RAM中的systemSettings，用于恢复设置值*/
bool restoreSettings() {
  // We read the flash
 /**
  * 整片Flash擦除后，首次烧入BOOTLDR，和用户APP后，
  * 0x0800C000起的setting_page未储存数据，每一个16bit块值为0xFFFF
  * 这时，从0x0800C000读到RAM中systemSettings的所有数据成员值为FF...
  */
	flash_read_buffer((uint8_t *)&systemSettings, sizeof(systemSettingsType));
#if	1
  // if the version is correct were done
  // if not we reset and save
/*
 * 那么判断满足，强制将内存块的systemSettings值恢复为默认值，并写入默认值到0x0800C000
 * 下一次时会加载0x0800C000保存过一次的值，不会再次resetSettings();
 */
  if (systemSettings.version != SETTINGSVERSION) {
    // probably not setup
    resetSettings();
    resetWatchdog();
    saveSettings();
    //osDelay(10);
    return true;
  }
#endif
  return false;
}
// Lookup function for cutoff setting -> X10 voltage
/*
 * 0=DC
 * 1=3S
 * 2=4S
 * 3=5S
 * 4=6S
 */
uint8_t lookupVoltageLevel() {
#if 0
  if (systemSettings.minDCVoltageCells == 0)
    return 90; // 9V since iron does not function effectively below this
  else
    return (systemSettings.minDCVoltageCells * 33) + (33 * 2);
#else
  return 90;
#endif
}
void resetSettings() {
  //memset((void *)&systemSettings, 0, sizeof(systemSettingsType));
  systemSettings.version           = SETTINGSVERSION;    // Store the version number to allow for easier upgrades

  systemSettings.AutoStartMode     = AUTO_START_MODE;    // Auto start off for safety
  systemSettings.MenuKeepHeating   = 0;
  systemSettings.SolderingTemp     = SOLDERING_TEMP;    // Default soldering temp is 320.0 C
  systemSettings.SleepTemp 		   = SLEEP_TEMP; 		// Temperature the iron sleeps at - 150.0 C
  systemSettings.ShutDownTemp      = SHUT_DOWN_TEMP;	//100.0 C
  systemSettings.TempOffsetRef     = TEMP_OffSET_REF;

  systemSettings.SleepMode		   = 1;
  systemSettings.SleepTime 		   = SLEEP_TIME; 				 // How many seconds to sleep
  systemSettings.ShutDownTime      = SHUTDOWN_TIME;      // How many seconds until the unit turns itself off after Sleep
  systemSettings.ShutDownMode	   = 1;

  systemSettings.ScreenBrightness  = 100;
  systemSettings.OrientationMode   = ORIENTATION_MODE;   // 0: Right 1:Left 2:Automatic - Default right

  systemSettings.Sensitivity       = SENSITIVITY;        // Default high sensitivity
  systemSettings.BuzzerVolume	   = 100;


  systemSettings.CalibrationOffset              = CALIBRATION_OFFSET;         // the adc offset in uV
  systemSettings.ResetForceDFU	   = 0;

  systemSettings.Language		   = LANGUAGES_DEFAULT;					 //0: Zh 1:En 2:Japan

  systemSettings.BoostTemp         = BOOST_TEMP;         // default to 400C
#ifdef ENABLED_FAHRENHEIT_SUPPORT
  systemSettings.temperatureInF = TEMPERATURE_INF; // default to 0
#endif
  systemSettings.descriptionScrollSpeed         = DESCRIPTION_SCROLL_SPEED;   // default to slow

  systemSettings.powerLimit                     = POWER_LIMIT;                // 65 watts default limit
  systemSettings.KeepAwakePulse                 = POWER_PULSE_DEFAULT;


  systemSettings.ReverseButtonTempChangeEnabled = REVERSE_BUTTON_TEMP_CHANGE; //
  systemSettings.TempChangeShortStep            = TEMP_CHANGE_SHORT_STEP;     //
  systemSettings.TempChangeLongStep             = TEMP_CHANGE_LONG_STEP;      //

  systemSettings.detailedSoldering = DETAILED_SOLDERING; // Detailed soldering screen
  //saveSettings(); // Save defaults
}
