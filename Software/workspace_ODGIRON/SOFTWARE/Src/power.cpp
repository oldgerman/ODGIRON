/*
 * power.cpp
 *
 *  Created on: 28 Oct, 2018
 *     Authors: Ben V. Brown, David Hilton <- Mostly David
 */

#include <BSP.h>
#include <Settings.h>
#include "power.hpp"

static int32_t PWMToX10Watts(uint8_t pwm, uint8_t sample);


//expMovingAverage<uint32_t, wattHistoryFilter> x10WattHistory = {0};
struct expMovingAverage x10WattHistory = {0};

int32_t tempToX10Watts(int32_t rawTemp) {
  // mass is in milliJ/*C, rawC is raw per degree C
  // returns milliWatts needed to raise/lower a mass by rawTemp
  //  degrees in one cycle.
  int32_t milliJoules = tipMass * rawTemp;
  return milliJoules;
}

void setTipX10Watts(int32_t mw) {
  int32_t output = X10WattsToPWM(mw, 1);	//sample只要1次，该函数跟了一圈结果返回0
  setTipPWM(output);						//默认pendingPWM为0
  uint32_t actualMilliWatts = PWMToX10Watts(output, 0);

  x10WattHistory.update(actualMilliWatts);//传入0
}

static uint32_t availableW10(uint8_t sample) {
  // P = V^2 / R, v*v = v^2 * 100 = (10*v)^2即getInputVoltageX10()返回值的是10倍的v
  //				R = R*10
  // P therefore is in V^2*100/R*10 = W*10.//那么算出的P是实际功率的10倍，可以灵活计到小数点后一位，避免浮点数
	//28.05V输入3.3V满偏，pwm = 32768(平均滤波后)*4/467=280.67=取280<--所以最大CCR1给了281？
	//24V输入，则：pwm = 28037(平均滤波后)*4/467=240.15=取240
	//12V输入，则：pwm = 14018(平均滤波后)*4/467=120.07=取120
	//返回值规律：100=10V
  uint32_t v                 = getInputVoltageX10(); // 100 = 10v
  uint32_t availableWattsX10 = (v * v) / tipResistance;	//TS100的烙铁头是7.5Ω，这里 / 75
  // However, 100% duty cycle is not possible as there is a dead time while the ADC takes a reading
  // Therefore need to scale available milliwats by this
  // 但是，由于ADC读取数据时会有空载时间，因此不可能实现100％的占空比
  // 因此需要对可用的毫瓦进行缩放

  // avMw=(AvMw*powerPWM)/totalPWM.
  availableWattsX10 = availableWattsX10 * powerPWM;
  availableWattsX10 /= totalPWM;	//totalPWM在switchToXXXPWM()中做修改,默认未初始化，是TIM2 ARR的值

  // availableMilliWattsX10 is now an accurate representation
  return availableWattsX10;		//默认是0
}

uint8_t X10WattsToPWM(int32_t milliWatts, uint8_t sample) {
  // Scale input milliWatts to the pwm range available
  //将输入毫瓦定为可用的pwm范围
 // 第一次进入时，milliWatts=0，只进行一次电源电压采集就返回
  if (milliWatts < 1) {
    // keep the battery voltage updating the filter
	// 更新滤波器以保持电池电压
	//在 settings.cpp中systemSettings.voltageDiv = VOLTAGE_DIV; // Default divider from schematic
	//#define VOLTAGE_DIV        467 // 467 - Default divider from schematic
	  //28.05V输入3.3V满偏，pwm = 32768(平均滤波后)*4/467=280.67=取280<--所以最大CCR1给了281？
	  //24V输入，则：pwm = 28037(平均滤波后)*4/467=240.15=取240
	  //12V输入，则：pwm = 14018(平均滤波后)*4/467=120.07=取120
    getInputVoltageX10();

    return 0;	//辛辛苦苦搞了这么久你return 0?
  }

  // Calculate desired milliwatts as a percentage of availableW10
  uint32_t pwm;
  do {
    pwm = (powerPWM * milliWatts) / availableW10(sample);
    if (pwm > powerPWM) {
      // constrain to max PWM counter, shouldn't be possible,
      // but small cost for safety to avoid wraps
      pwm = powerPWM;
    }
  } while (tryBetterPWM(pwm));

  return pwm;
}

static int32_t PWMToX10Watts(uint8_t pwm, uint8_t sample) {
  uint32_t maxMW = availableW10(sample);  // 获得最大ARR下pwm的毫瓦
  	  	  	  	  	  	  	  	  	  	  // Get the milliwatts for the max pwm period
  // Then convert pwm into percentage of powerPWM to get the percentage of the max mw
  // 然后将pwm转换为powerPWM的百分比以获得最大mw的百分比
  return (((uint32_t)pwm) * maxMW) / powerPWM;//返回0
}
