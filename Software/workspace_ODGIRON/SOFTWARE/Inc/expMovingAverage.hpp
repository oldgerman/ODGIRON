/*
 * expMovingAverage.h
 *
 *  Created on: 8 Oct 2019
 *      Author: ralim
 */
//#program once
#ifndef INC_EXPMOVINGAVERAGE_HPP_
#define INC_EXPMOVINGAVERAGE_HPP_

#if 0
// max size = 127
template <class T, uint8_t weighting>
struct expMovingAverage {
  int32_t sum;
  void    update(T const val) { sum = ((val * weighting) + (sum * (256 - weighting))) / 256; }

  T average() const { return sum; }
};

#endif
#ifdef __cplusplus
extern "C" {
#endif
#include "power.hpp"

#ifdef __cplusplus

struct expMovingAverage {
  int32_t sum;
  void    update(uint32_t const val) { sum = ((val * wattHistoryFilter) + (sum * (256 - wattHistoryFilter))) / 256; }

  uint32_t average() const { return sum; }
};
}
#endif

#endif /* INC_EXPMOVINGAVERAGE_HPP_ */
