/*
 * Colum.cpp
 *
 *  Created on: 2021年4月20日
 *      Author: OldGerman
 */

#include <Colum.hpp>

/*
 * 如果需要在Page内部的for(;;)的switch (buttons) {...}更改AutoValue对象的值，需要使用这种写法：
 * buttons = getButtonState();
 * AutoValue::buttonState = buttons; //	加这一句
 * switch (buttons) {
 * 	case BUTTON_B_SHORT:
 * 		(*ptrAutoValue)--; //AutoValue::operator--()在内部判断buttons长按短按，ptrAutoValue是指向AutoValue类型的指针
 * 	case BUTTON_F_SHORT:
 * 		(*ptrAutoValue)++;
 * }
 * 否则，不需要AutoValue::buttonState = buttons;这句
 */
ButtonState  AutoValue::buttonState;

