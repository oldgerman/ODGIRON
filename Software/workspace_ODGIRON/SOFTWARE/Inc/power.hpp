/*
 * Power.hpp
 *
 *  Created on: 28 Oct, 2018
 *     Authors: Ben V. Brown, David Hilton (David's Idea)
 */

#include <expMovingAverage.hpp>
#include <history.hpp>
#include "configuration.h"
#include "BSP.h"
#include "stdint.h"
#ifndef POWER_HPP_
#define POWER_HPP_

// thermal mass = 1690 milliJ/*C for my tip.
//  ->  Wattsx10*Seconds to raise Temp from room temp to +100*C, divided by 100*C.
// we divide mass by 20 to let the I term dominate near the set point.
//  This is necessary because of the temp noise and thermal lag in the system.
// Once we have feed-forward temp estimation we should be able to better tune this.
// 热容= 1690 milliJ / * C。对于我的小烙铁头
// -> Wattsx10 *秒，将Temp从室温提高到+ 100 * C，再除以100 * C。
// 我们将质量除以20，以使I项在设定点附近占主导地位。
// 这是必需的，因为系统中存在温度噪声和热滞后。
// 一旦获得前馈温度估计，我们应该能够更好地进行调整。
// 热质量等同于热容，一个主体的存储能力的热能。它通常是指由符号Cth 表示，以J/℃或J/K为单位计量

#ifdef __cplusplus

extern "C" {
#endif
const uint8_t                                        wattHistoryFilter = 24; // I term look back weighting																//我指定的历史权重
int32_t tempToX10Watts(int32_t rawTemp);
void    setTipX10Watts(int32_t mw);


#ifdef __cplusplus
//extern expMovingAverage<uint32_t, wattHistoryFilter> x10WattHistory;
extern struct expMovingAverage x10WattHistory;
uint8_t X10WattsToPWM(int32_t milliWatts, uint8_t sample = 0);	//默认参数值是C++特性
}
#endif

#endif /* POWER_HPP_ */
