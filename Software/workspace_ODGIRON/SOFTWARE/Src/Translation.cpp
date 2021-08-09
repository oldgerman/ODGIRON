// WARNING: THIS FILE WAS AUTO GENERATED BY make_translation.py. PLEASE DO NOT EDIT.

#include "Translation.h"
#ifndef LANG
#define LANG_EN
#endif

#ifdef LANG_EN
// ---- English ----

const char* SettingsDescriptions[] = {
  /* PowerSource               */ "Power source. Sets cutoff voltage. <DC 10V> <S 3.3V per cell>",
  /* SleepTemperature          */ "Sleep Temperature <C>",
  /* SleepTimeout              */ "Sleep Timeout <Minutes/Seconds>",
  /* ShutdownTimeout           */ "Shutdown Timeout <Minutes>",
  /* MotionSensitivity         */ "Motion Sensitivity <0.Off 1.least sensitive 9.most sensitive>",
  /* TemperatureUnit           */ "Temperature Unit <C=Celsius F=Fahrenheit>",
  /* AdvancedIdle              */ "Display detailed information in a smaller font on the idle screen.",
  /* DisplayRotation           */ "Display Orientation <A. Automatic L. Left Handed R. Right Handed>",
  /* BoostEnabled              */ "Enable front key enters boost mode 450C mode when soldering",
  /* BoostTemperature          */ "Temperature when in \"boost\" mode",
  /* AutoStart                 */ "Automatically starts the iron into soldering on power up. T=Soldering, S= Sleep mode,F=Off",
  /* CooldownBlink             */ "Blink the temperature on the cooling screen while the tip is still hot.",
  /* TemperatureCalibration    */ "Calibrate tip offset.",
  /* SettingsReset             */ "Reset all settings",
  /* VoltageCalibration        */ "VIN Calibration. Buttons adjust, long press to exit",
  /* AdvancedSoldering         */ "Display detailed information while soldering",
  /* ScrollingSpeed            */ "Speed this text scrolls past at",
  /* TipModel                  */ "Tip Model selection",
  /* SimpleCalibrationMode     */ "Simple Calibration using Hot water",
  /* AdvancedCalibrationMode   */ "Advanced calibration using thermocouple on the tip",
  /* PowerInput                */ "Power Wattage of the power adapter used",
};

const char* SettingsCalibrationDone = "Calibration done!";
const char* SettingsCalibrationWarning = "Please ensure the tip is at room temperature before continuing!";
const char* SettingsResetWarning = "Are you sure you want to reset settings to default values?";
const char* UVLOWarningString = "DC LOW";
const char* UndervoltageString = "Undervoltage";
const char* InputVoltageString = "Input V: ";
const char* WarningTipTempString = "Tip Temp: ";
const char* BadTipString = "BAD TIP";
const char* SleepingSimpleString = "Zzzz";
const char* SleepingAdvancedString = "Sleeping...";
const char* WarningSimpleString = "HOT!";
const char* WarningAdvancedString = "!!! TIP HOT !!!";
const char* SleepingTipAdvancedString = "Tip:";
const char* IdleTipString = "Tip:";
const char* IdleSetString = " Set:";
const char* TipDisconnectedString = "PULLED OUT";
const char* SolderingAdvancedPowerPrompt = "Power: ";
const char* OffString = "Off";

const char* SettingTrueChar = "T";
const char* SettingFalseChar = "F";
const char* SettingRightChar = "R";
const char* SettingLeftChar = "L";
const char* SettingAutoChar = "A";
const char* SettingFastChar = "F";
const char* SettingSlowChar = "S";

//const enum ShortNameType SettingsShortNameType = SHORT_NAME_DOUBLE_LINE;
const char* SettingsShortNames[][2] = {
  /* PowerSource               */ { "Power", "source" },
  /* SleepTemperature          */ { "Sleep", "temp" },
  /* SleepTimeout              */ { "Sleep", "timeout" },
  /* ShutdownTimeout           */ { "Shutdown", "timeout" },
  /* MotionSensitivity         */ { "Motion", "sensitivity" },
  /* TemperatureUnit           */ { "Temperature", "units" },
  /* AdvancedIdle              */ { "Detailed", "idle screen" },
  /* DisplayRotation           */ { "Display", "orientation" },
  /* BoostEnabled              */ { "Boost mode", "enabled" },
  /* BoostTemperature          */ { "Boost", "temp" },
  /* AutoStart                 */ { "Auto", "start" },
  /* CooldownBlink             */ { "Cooldown", "blink" },
  /* TemperatureCalibration    */ { "Calibrate", "temperature?" },
  /* SettingsReset             */ { "Factory", "Reset?" },
  /* VoltageCalibration        */ { "Calibrate", "input voltage?" },
  /* AdvancedSoldering         */ { "Detailed", "solder screen" },
  /* ScrollingSpeed            */ { "Scrolling", "Speed" },
  /* TipModel                  */ { "Tip", "Model" },
  /* SimpleCalibrationMode     */ { "Simple", "Calibration" },
  /* AdvancedCalibrationMode   */ { "Advanced", "Calibration" },
  /* PowerInput                */ { "Power", "Wattage" },
};

const char* SettingsMenuEntries[4] = {
  /* SolderingMenu             */ "Soldering\nSettings",
  /* PowerSavingMenu           */ "Sleep\nModes",
  /* UIMenu                    */ "User\nInterface",
  /* AdvancedMenu              */ "Advanced\nOptions",
};

const char* SettingsMenuEntriesDescriptions[4] = {
  /* SolderingMenu             */ "Soldering settings",
  /* PowerSavingMenu           */ "Power saving settings",
  /* UIMenu                    */ "User interface settings",
  /* AdvancedMenu              */ "Advanced options",
};



//以下从Translations/make_translation.py得到
//从getDebugMenu()构建

const char *DebugMenu[] = {
	//SymbolVersionNumber,
	"ironOS-based PD T12",
	__DATE__ " " __TIME__,	// 编译日期
	"Design by: OldGerman", // Print version number
	"github.com/oldgerman",
    "HW GUI ",  //GUI任务的高水位标记 // High Water marker for GUI task
    "HW MOV ",  //MOV任务的高水位标记 // High Water marker for MOV task
    "HW PID ",  //PID任务的高水位标记 // High Water marker for PID task
    "HW POW ",  //GUI任务的高水位标记 // High Water marker for GUI task

    "HW INT ",  //GUI任务的高水位标记 // High Water marker for GUI task
    "HW PEG ",  //MOV任务的高水位标记 // High Water marker for MOV task
    "HW RX ",  	//PID任务的高水位标记 // High Water marker for PID task
    "HW TX ",  	//GUI任务的高水位标记 // High Water marker for GUI task

    "Time ",  	// Uptime (aka timestamp,
    "PCB  ",  	// PCB Version AKA IMU version
    "Move ",  	// Time of last significant movement
	"Button ",  // Time of last significant movement

    "mVTip",  	// Tip reading in uV
    "CTip ",  	// Tip temp in C
    "CHanx10",  // Handle temp in C
    "CMax  ",   // Max deg C limit

    "PWR Mode", // Power Negotiation State
"VINx10",  		// Input voltage
	"LCPx1000", // Load Current Produce
	"Calibrate"	//systemSettings.calibration
};
//从 getConstants() 构建
const char *SymbolPlus = "+";
const char *SymbolMinus = "-";
const char *SymbolSpace = " ";
const char *SymbolDot = ".";
const char *SymbolDegC = "C";
const char *SymbolDegF = "F";
const char *SymbolMinutes = "M";
const char *SymbolSeconds = "S";
const char *SymbolWatts = "W";
const char *SymbolVolts = "V";
const char *SymbolDC = "DC";
const char *SymbolCellCount = "S";
const char *SymbolVersionNumber = "    ODGIRON v1.0";		//自己加的



//以下从Translations/translation_en.json "messages"得到
const char *ResetOKMessage = "Reset OK";
const char *YourGainMessage = "Your gain:";
const char *SettingsResetMessage = "Settings were\nreset!";
const char *NoAccelerometerMessage = "No accelerometer\ndetected!";
const char *NoPowerDeliveryMessage = "No USB-PD IC\ndetected!";
const char *LockingKeysString = "LOCKED";
const char *UnlockingKeysString = "UNLOCKED";
const char *WarningKeysLockedString = "!LOCKED!";
const char *SelfCheckIsOK= "Self check completed!\nPress any key to\ncontinue...";

//以下从Translations/translation_en.json  "characters"得到
const char *SettingStartSolderingChar = "S";
const char *SettingStartSleepChar = "Z";
const char *SettingStartSleepOffChar = "R";
const char *SettingStartNoneChar = "O";
const char *SettingSensitivityOff = "O";
const char *SettingSensitivityLow = "L";
const char *SettingSensitivityMedium = "M";
const char *SettingSensitivityHigh = "H";
const char *SettingLockDisableChar = "D";
const char *SettingLockBoostChar = "B";
const char *SettingLockFullChar = "F";
#endif

