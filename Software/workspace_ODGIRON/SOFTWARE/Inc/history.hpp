/*
 * history.hpp
 *
 *  Created on: 28 Oct, 2018
 *     Authors: Ben V. Brown, David Hilton
 */

#ifndef HISTORY_HPP_
#define HISTORY_HPP_


#include <stdint.h>

#if 0
// max size = 127
template<class T, uint8_t SIZE>
struct history {
	static const uint8_t size = SIZE;	//static const 类成员，为所有类的实例共享的数据，必须在类内对其初始化
	T buf[size];
	int32_t sum;	//T类型值的总和
	uint8_t loc;	//元素编号（location）

	void update(T const val) {
		// step backwards so i+1 is the previous value.
		//向后退一步，因此i + 1是前一个值。

		sum -= buf[loc];
		sum += val;
		buf[loc] = val;
		loc = (loc + 1) % size;	//loc0+8次1才会使loc=1？
	}

	//下标运算符 [] 重载
	T operator[](uint8_t i) const {
		// 0 = newest, size-1 = oldest.
		i = (i + loc) % size;
		return buf[i];
	}

	T average() const {
		return sum / size;
	}
};
#endif



#endif  /* HISTORY_HPP_ */
