#include "BSP_Flash.h"
#include "BSP_Power.h"
#include "Defines.h"
#include "Model_Config.h"
#include <stdbool.h>
#include <stdint.h>

#include "I2C_Wrapper.h"
/*
 * BSP.h -- Board Support
 *
 * This exposes functions that are expected to be implemented to add support for different hardware
 */

#ifndef BSP_BSP_H_
#define BSP_BSP_H_
#ifdef __cplusplus
extern FRToSI2C FRToSI2C1;
extern FRToSI2C FRToSI2C2;
extern "C" {
#endif

extern bool fusb302_process;
extern volatile uint16_t PWMSafetyTimer;
extern volatile uint8_t  pendingPWM;
// maximum htim2 PWM value
extern const uint16_t powerPWM;
// htim2.Init.Period, the full PWM cycle
extern uint16_t totalPWM;

// Called first thing in main() to init the hardware
void preRToSInit();
// Called once the RToS has started for any extra work
void postRToSInit();

// Called once from preRToSInit()
void BSPInit(void);

// Called to reset the hardware watchdog unit
void resetWatchdog();
// Accepts a output level of 0.. to use to control the tip output PWM
void setTipPWM(uint8_t pulse);
// Returns the Handle temp in C, X10
uint16_t getHandleTemperature();
// Returns the Tip temperature ADC reading in raw units
uint16_t getTipRawTemp(uint8_t refresh);
// Returns the main DC input voltage, using the adjustable divisor + sample flag
uint16_t getInputVoltageX10();
// Switch to the most suitable PWM freq given the desired period;
// returns true if the switch was performed and totalPWM changed
bool tryBetterPWM(uint8_t pwm);

// Readers for the two buttons
// !! Returns 1 if held down, 0 if released
uint8_t getButtonA();
uint8_t getButtonB();
uint8_t getButtonOK();

// This is a work around that will be called if I2C starts to bug out
// This should toggle the SCL line until SDA goes high to end the current transaction
void unstick_I2C(I2C_HandleTypeDef *);

// Reboot the IC when things go seriously wrong
void reboot();

// If the user has programmed in a bootup logo, draw it to the screen from flash
// Returns 1 if the logo was printed so that the unit waits for the timeout or button
uint8_t showBootLogoIfavailable();
// delay wrapper for delay using the hardware timer (used before RTOS)
void delay_ms(uint16_t count);
// Used to allow knowledge of if usb_pd is being used
uint8_t usb_pd_detect();
bool    getHallSensorFitted();
// If the iron has a hall effect sensor in the handle, return an signed count of the reading
// If the sensor is single polarity (or polarity insensitive) just return 0..32768
int16_t getRawHallEffect();

// Returns true if power is from dumb "DC" input rather than "smart" QC or PD
// 如果电源来DC输入而不是QC或PD，则返回true
bool getIsPoweredByDCIN();

#ifdef __cplusplus
}
#endif
#endif /* BSP_BSP_H_ */
