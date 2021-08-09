/*
 * cppports.h
 *
 *  Created on: Jun 10, 2021
 *      Author: PSA
 */

#ifndef INC_CPPPORTS_H_
#define INC_CPPPORTS_H_
#include "oled_init.h"
#include "main.h"
#include "usb_device.h"
#include "Settings.h"
#ifdef __cplusplus

class AutoValue
{
public:
	//引用在定义时必须初始化
	AutoValue(){}
	AutoValue(uint16_t *Val, uint8_t Places, uint16_t Upper, uint16_t Lower, uint8_t ShortSteps, uint8_t LongSteps = 0, bool FollowButtonState = true)
	:val(Val), places(Places), upper(Upper), lower(Lower), shortSteps(ShortSteps), longSteps(LongSteps), followButtonState(FollowButtonState)
	{
		percentageMode = false; //默认不使用百分比模式
	}
	virtual ~AutoValue() {}
	uint16_t operator++(int) {
		if((buttonState == BUTTON_F_SHORT || !followButtonState) && shortSteps != 0)
			stepsPlus(shortSteps);
		//++运算不足一个LongSteps时会自动变为upper
		if(buttonState == BUTTON_F_LONG || shortSteps == 0)
			stepsPlus(longSteps);
		return *val;
	}

	uint16_t & operator--(int) {
		if ((buttonState == BUTTON_B_SHORT || !followButtonState)
				&& shortSteps != 0)
			stepsLess(shortSteps);
		//--运算不足一个LongSteps时会自动变为lower
		else if (buttonState == BUTTON_B_LONG || shortSteps == 0)
			stepsLess(longSteps);
		else
			;
		return *val;
	}

	bool valueIsBool() {
		return (upper == 1 && lower == 0) ? 1 : 0;
	}

	void stepsLess(uint8_t steps)
		{
			if(*val - lower > steps)
				*val -= steps;
			else
				*val = lower;
		}
	void stepsPlus(uint8_t steps)
		{
			if(upper - *val > steps)
				*val += steps;
			else
				*val = upper;
		}
	uint16_t *val;	//待修改值
	uint8_t places;	//值的位数, //再说。若为浮点类则小数点占一位
	uint16_t upper;	//值的上限
	uint16_t lower;	//值的下限
	uint8_t shortSteps;	//短按值的步幅
	uint8_t longSteps;	//长按值的步幅
	bool followButtonState;	//是否跟随buttons状态，默认跟随，例外是Page::indexColums不跟随，会导致非法访问
	static ButtonState buttonState;
	bool percentageMode;
	uint16_t percentageVal;
};

extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* 私有包含 Private includes -------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* 导出类型 Exported types ---------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef  void (*pFunction)(void);
/* USER CODE END ET */

/* 导出常量 Exported constants -----------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* 导出的宏 Exported macro ---------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* 导出函数原型 Exported functions prototypes --------------------------------*/

/* USER CODE BEGIN EFP */
void doGUIWork();
void checkAPPJumper();
void GUIDelay();
/* USER CODE END EFP */

/* 私有定义 Private defines --------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* INC_CPPPORTS_H_ */
