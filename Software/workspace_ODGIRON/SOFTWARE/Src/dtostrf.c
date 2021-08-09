/*
  dtostrf - Emulation for dtostrf function from avr-libc
  Copyright (c) 2013 Arduino.  All rights reserved.
  Written by Cristian Maglie <c.maglie@bug.st>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "dtostrf.h"

/**
 * 把double型数据转换成char型字符串
 * 参数“__val”：需要转换的double型数据；
 * 参数“s”：转换成的字符串首地址；
 * 参数“__width”：输出字符串的总位数；小数点也算一位
 * 参数“__prec”：输出字符串小数点后的位数
 *
 * double数据转换成char字符串的函数——dtostrf()用法:
 * https://blog.csdn.net/electrocrazy/article/details/52817193)
 */
char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
    char fmt[20];
    sprintf(fmt, "%%%d.%df", width, prec);
    sprintf(sout, fmt, val);
    return sout;
}

