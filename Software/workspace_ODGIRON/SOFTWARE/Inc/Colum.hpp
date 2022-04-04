/*
 * Colum.hpp
 *
 *  Created on: 2021年4月20日
 *      Author: OldGerman
 */

#ifndef INC_COLUM_HPP_
#define INC_COLUM_HPP_
#include "stdint.h"
#include "Buttons.hpp"	//提供buttons

#ifdef __cplusplus
#include <map>
//改值页面函数指针执行位置
enum FunLoc {
	LOC_NONE,   //不执行
	LOC_ENTER,	//刚进入函数，执行一次
	LOC_CHANGE,	//一旦修改值就执行
	LOC_EXTI	//推出函数，执行一次
};

class Page;
/**
 * 对于ON/OFF，则val传入bool，upper传入1， lower传入0
 * 待实现：值是否循环
 */

class AutoValue {
public:
	//引用在定义时必须初始化
	AutoValue() {
	}
	AutoValue(uint16_t *Val, uint8_t Places, uint16_t Upper, uint16_t Lower,
			uint8_t ShortSteps, uint8_t LongSteps = 0, bool FollowButtonState =
					true, bool ValueCycle = false) :
			val(Val), places(Places), upper(Upper), lower(Lower), shortSteps(
					ShortSteps), longSteps(LongSteps), followButtonState(
					FollowButtonState), valueCycle(ValueCycle) {
		if (FollowButtonState)
			valueCycle = true;
		//percentageMode = false; //默认不使用百分比模式
	}
	virtual ~AutoValue() {
	}
	uint16_t operator++(int) {
		if ((buttonState == BUTTON_F_SHORT || !followButtonState)
				&& shortSteps != 0)
			stepsPlus(shortSteps);
		//++运算不足一个LongSteps时会自动变为upper
		if (buttonState == BUTTON_F_LONG || shortSteps == 0)
			stepsPlus(longSteps);
		return *val;
	}

	uint16_t& operator--(int) {
		if ((buttonState == BUTTON_B_SHORT || !followButtonState)
				&& shortSteps != 0)
			stepsLess(shortSteps);
		//--运算不足一个LongSteps时会自动变为lower
		if (buttonState == BUTTON_B_LONG || shortSteps == 0)
			stepsLess(longSteps);
		return *val;
	}

	bool valueIsBool() {
		return (upper == 1 && lower == 0) ? 1 : 0;
	}

	void stepsLess(uint8_t steps) {
		if (*val - lower > steps)
			*val -= steps;
		else {
			*val = lower;
			if (valueCycle) {	//循环值
				static uint8_t bound = 0;
				++bound;
				if (bound == 2) {
					*val = upper;
					bound = 0;
				}
			}
		}
	}
	void stepsPlus(uint8_t steps) {
		if (upper - *val > steps)
			*val += steps;
		else {
			*val = upper;
			if (valueCycle) {	//循环值
				static uint8_t bound = 0;
				++bound;
				if (bound == 2) {
					*val = lower;
					bound = 0;
				}
			}
		}
	}
	uint16_t *val;	//待修改值
	uint8_t places;	//值的位数
	uint16_t upper;	//值的上限
	uint16_t lower;	//值的下限
	uint8_t shortSteps;	//短按值的步幅
	uint8_t longSteps;	//长按值的步幅
	bool followButtonState;	//是否跟随buttons状态，默认跟随，例外是Page::indexColums不跟随，会导致非法访问
	static ButtonState buttonState;	//用于从外部获取按钮状态
	uint16_t percentageVal;
	bool valueCycle;		//值是否越界循环
};

class Colum {
public:
	Colum(const char *Str = nullptr) :
			str(Str) {
		ptrAutoValue = nullptr;
		unit = nullptr;
		nextPage = nullptr;
		funPtr = nullptr;
		ptrColumVal2Str = nullptr;
	}
	Colum(const char *Str, uint16_t *Val, uint8_t Places, uint16_t Upper,
			uint16_t Lower, uint8_t ShortSteps, uint8_t LongSteps = 0,
			const char *Uint = nullptr, void (*FunPtr)(void) = nullptr,
			FunLoc FunLoc = LOC_NONE,
			std::map<uint16_t, const char*> *PtrColumVal2Str = nullptr) :
			str(Str), unit(Uint), funPtr(FunPtr), funLoc(FunLoc), ptrColumVal2Str(
					PtrColumVal2Str) {
		ptrAutoValue = new AutoValue(Val, Places, Upper, Lower, ShortSteps,
				LongSteps);
		nextPage = nullptr;
	}
	Colum(const char *Str, AutoValue *PtrAutoValue, const char *Uint = nullptr,
			void (*FunPtr)(void) = nullptr, FunLoc FunLoc = LOC_NONE,
			std::map<uint16_t, const char*> *PtrColumVal2Str = nullptr) :
			str(Str), ptrAutoValue(PtrAutoValue), unit(Uint), funPtr(FunPtr), funLoc(
					FunLoc), ptrColumVal2Str(PtrColumVal2Str) {
		nextPage = nullptr;
	}
	Colum(const char *Str, void (*FunPtr)(void), FunLoc FunLoc = LOC_NONE) :
			str(Str), funPtr(FunPtr), funLoc(FunLoc) {
		ptrAutoValue = nullptr;
		nextPage = nullptr;
		ptrColumVal2Str = nullptr;
	}
	//仅这个函数nextPage ≠ nullptr，该Colum对象进入下一个Page对象，三级及以上菜单使用
	Colum(const char *Str, Page *NextPage) :
			str(Str), nextPage(NextPage) {
		ptrAutoValue = nullptr;
		unit = nullptr;
		funPtr = nullptr;
		ptrColumVal2Str = nullptr;
	}

	virtual ~Colum() {
	}

	//warning：str will be initialized after：确保成员在初始化列表中的出现顺序与在类中出现的顺序相同：
	const char *str;
	AutoValue *ptrAutoValue;
	const char *unit;
	Page *nextPage;
	Page *prevPage;
	//bool x10;	//对于浮点数用X10倍
	void (*funPtr)(void);	//改值页执行函数
	FunLoc funLoc;
	std::map<uint16_t, const char*> *ptrColumVal2Str;//若有数值映射需要，则指向传入的map数组解析值到字符串
};
#endif
#endif /* INC_COLUM_HPP_ */
