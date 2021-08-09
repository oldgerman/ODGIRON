/*
 * Buttons.h
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 *      Modify: OldGerman
 */
#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include "main.h"

#ifdef __cplusplus

// Returns what buttons are pressed (if any)


// Helpers
void waitForButtonPressOrTimeout(uint32_t timeout);
void waitForButtonPress();

extern "C" {
#endif

//7种状态测试OK
typedef enum{
  BUTTON_NONE      = 0,  /* No buttons pressed / < filter time*/
  BUTTON_F_SHORT   = 1,  /* User has pressed the front button*/
  BUTTON_B_SHORT   = 2,  /* User has pressed the back  button*/
  BUTTON_F_LONG    = 4,  /* User is  holding the front button*/
  BUTTON_B_LONG    = 8,  /* User is  holding the back button*/
  BUTTON_BOTH      = 16, /* User has pressed both buttons*/
  BUTTON_BOTH_LONG = 32, /* User is holding both buttons*/
  BUTTON_OK_SHORT  = 64, /* OK键短按 */
  BUTTON_OK_LONG   = 128,/* OK键长按 */
  BUTTON_IDLE	   = 200,/* 用于不同Page间切换时提供断点*/
  BUTTON_FROZEN    = 233 /*用于切换时的冻结时间*/

  /*
   * Note:
   * Pressed means press + release, we trigger on a full \__/ pulse
   * holding means it has gone low, and been low for longer than filter time
   */
}ButtonState;
extern ButtonState buttons;
extern bool buttonChanged;
extern bool buttonIsBeep;
extern uint32_t lastButtonTime;
uint8_t getButtonA();
uint8_t getButtonB();
uint8_t getButtonOK();
extern bool waitingToChooseOneFromTwo();
ButtonState readButtonState();
ButtonState getButtonState();
#ifdef __cplusplus
}
#endif
#endif /* INC_BUTTONS_H_ */
