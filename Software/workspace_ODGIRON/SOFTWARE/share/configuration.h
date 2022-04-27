#pragma once
#include "Model_Config.h"
#include <stdint.h>
/**
 * Configuration.h
 * Define here your default pre settings for TS80 or TS100
 *
 */
#define TICKS_SECOND configTICK_RATE_HZ
#define TICKS_MIN    (60 * TICKS_SECOND)
#define TICKS_100MS  (TICKS_SECOND / 10)
#define TICKS_10MS   (TICKS_100MS / 10)

//===========================================================================
//============================= Default Settings ============================
//===========================================================================
#define LANGUAGES_NUM     		 3	//语言数量
#define LANGUAGES_DEFAULT      0	//默认语言为中文
/**
 * Default soldering temp is 320.0 C
 * Temperature the iron sleeps at - default 150.0 C
 */
#define SOLDERING_TEMP     320 // Default soldering temp is 320.0 °C
#define SLEEP_TEMP         200 // Default sleep temperature
#define SHUT_DOWN_TEMP     100 // Default sleep temperature
#define TEMP_OffSET_REF    0
#define BOOST_TEMP         420 // Default boost temp.
#define BOOST_MODE_ENABLED 1   // 0: Disable 1: Enable

/**
 * Blink the temperature on the cooling screen when its > 50C
 */
#define COOLING_TEMP_BLINK 0 // 0: Disable 1: Enable

/**
 * How many seconds/minutes we wait until going to sleep/shutdown.
 * Values -> SLEEP_TIME * 10; i.e. 5*10 = 50 Seconds!
 */
#define SLEEP_TIME    300  // Seconds
#define SHUTDOWN_TIME 300 // Sec

/**
 * Auto start off for safety.
 * Pissible values are:
 *  0 - none
 *  1 - Soldering Temperature
 *  2 - Sleep Temperature
 *  3 - Sleep Off Temperature
 */
#define AUTO_START_MODE 1 // Default to none

/**
 * Locking Mode
 * When in soldering mode a long press on both keys toggle the lock of the buttons
 * Possible values are:
 *  0 - Desactivated
 *  1 - Lock except boost
 *  2 - Full lock
 */
#define LOCKING_MODE 2 // Default to desactivated for safety//为了安全起见，默认禁用

/**
 * OLED Orientation
 *
 */
#define ORIENTATION_MODE           0 // 0: Right 1:Left 2:Automatic - Default right
#define REVERSE_BUTTON_TEMP_CHANGE 0 // 0:Default 1:Reverse - Reverse the plus and minus button assigment for temperatur change

/**
 * Temp change settings
 */
#define TEMP_CHANGE_SHORT_STEP     1  // Default temp change short step +1
#define TEMP_CHANGE_LONG_STEP      10 // Default temp change long step +10
#define TEMP_CHANGE_SHORT_STEP_MAX 50 // Temp change short step MAX value
#define TEMP_CHANGE_LONG_STEP_MAX  90 // Temp change long step MAX value

/* Power pulse for keeping power banks awake*/
#define POWER_PULSE_INCREMENT 1
#define POWER_PULSE_MAX       50 // x10 max watts

/**
 * OLED Orientation Sensitivity on Automatic mode!
 * Motion Sensitivity <0=Off 1=Least Sensitive 9=Most Sensitive>
 */
#define SENSITIVITY 20 // Default 20%

/**
 * Detailed soldering screen
 * Detailed idle screen (off for first time users)
 */
#define DETAILED_SOLDERING 0 // 0: Disable 1: Enable - Default 0
#define DETAILED_IDLE      0 // 0: Disable 1: Enable - Default 0

#define CUT_OUT_SETTING          0 // default to no cut-off voltage
#define TEMPERATURE_INF          0 // default to 0
#define DESCRIPTION_SCROLL_SPEED 0 // 0: Slow 1: Fast - default to slow

//STAGE:运放增益 317
#define OP_AMP_Rf_TS100  750 * 1000 // 750  Kilo-ohms -> From schematic, R1
#define OP_AMP_Rin_TS100 2400       // 2.4 Kilo-ohms -> From schematic, R2
#define OP_AMP_GAIN_STAGE_TS100 (1 + (OP_AMP_Rf_TS100 / OP_AMP_Rin_TS100))

#define OP_AMP_Rf_TS80  180 * 1000 //  180  Kilo-ohms -> From schematic, R6
#define OP_AMP_Rin_TS80 2000       //  2.0  Kilo-ohms -> From schematic, R3
#define OP_AMP_GAIN_STAGE_TS80 (1 + (OP_AMP_Rf_TS80 / OP_AMP_Rin_TS80))

// Deriving the Voltage div:
// Vin_max = (3.3*(r1+r2))/(r2)
// vdiv = (32768*4)/(vin_max*10)

#if defined(MODEL_TS100)
#define VOLTAGE_DIV        467 // 467 - Default divider from schematic
#define CALIBRATION_OFFSET 900 // 900 - Default adc offset in uV
#define PID_POWER_LIMIT    70  // Sets the max pwm power limit
#define POWER_LIMIT        0   // 0 watts default limit
#define MAX_POWER_LIMIT    65  //
#define POWER_LIMIT_STEPS  5   //
#define OP_AMP_GAIN_STAGE  OP_AMP_GAIN_STAGE_TS100
#define TEMP_uV_LOOKUP_HAKKO
#endif


#if defined(MODEL_TS80P)
#if defined(ODGIRON)!=1
#define VOLTAGE_DIV        650  // Default for TS80P with slightly different resistors
#define PID_POWER_LIMIT    35   // Sets the max pwm power limit
#define CALIBRATION_OFFSET 1500 // the adc offset in uV
#define POWER_LIMIT        30   // 30 watts default power limit
#define MAX_POWER_LIMIT    35   //
#define POWER_LIMIT_STEPS  2
#define OP_AMP_GAIN_STAGE  OP_AMP_GAIN_STAGE_TS80
#define TEMP_uV_LOOKUP_TS80
#else	//ODGIRON
#define HARDWARE_MAX_WATTAGE_X10 750
#define TIP_THERMAL_MASS         65 // X10 watts to raise 1 deg C in 1 second
#define TIP_RESISTANCE           81 // x10 ohms, 7.5 typical for ts100 tips
#define THERMAL_RUNAWAY_TIME_SEC 20
#define THERMAL_RUNAWAY_TEMP_C   10

#define POWER_PULSE_DEFAULT 1
#define POWER_PULSE_WAIT_DEFAULT     4 // Default rate of the power pulse: 4*2500 = 10000 ms = 10 s
#define POWER_PULSE_DURATION_DEFAULT 1 // Default duration of the power pulse: 1*250 = 250 ms


#define VOLTAGE_DIV        467 // 467 - Default divider from schematic
//噢噢噢。他这里用了开路不加热上拉39K电阻到3.9V，给ADC一个偏置电压
#define CALIBRATION_OFFSET 750 // 900 - Default adc offset in uV //但是4096中，1 digit = 805uV
#define PID_POWER_LIMIT    70  // Sets the max pwm power limit
#define POWER_LIMIT        50   // 0 watts default limit
#define MAX_POWER_LIMIT    65  //
#define POWER_LIMIT_STEPS  5   //
#define OP_AMP_GAIN_STAGE  OP_AMP_GAIN_STAGE_TS100
#define TEMP_uV_LOOKUP_HAKKO
#endif

#endif

#if defined(ODGIRON)!=1
const uint32_t tipMass       = 40;
const uint8_t  tipResistance = 45; // x10 ohms, 4.5 typical for ts80 tips
#else	//ODGIRON
const int32_t tipMass       = 65; // X10 watts to raise 1 deg C in 1 second
const uint8_t tipResistance = 82; // x10 ohms, 8.0 typical for T12 tips
#endif
