/*
 * BSP_PD.c
 *
 *  Created on: 21 Jul 2020
 *      Author: Ralim
 */

#include "BSP_PD.h"
#include "Model_Config.h"
#ifdef POW_PD
/*
 * An array of all of the desired voltages & minimum currents in preferred order
 */
const uint16_t USB_PD_Desired_Levels[] = {
    // 每一挡位的最小电流需求，取烙铁头8Ω
	20000, 2250, // 20V @ 2.25A	//MAX 50.0W
	15000, 2000, // 15V @ 3.0A	//MAX 28.1W
	12000, 2250, // 12V @ 3.0A	//MAX 27.0W
	11000, 2000, // 10V @ 2A	//MAX 20.8W
	10000, 1600, // 10V @ 2A	//MAX 15.6W
	 9000, 1200, // 9V @ 2A		//MAX 11.4W
#if 0
    20000, 3250, // 20V @ 3.25A	//40W，小米PD电源 65W取电正常  100%占空比最大电流2000mA PID最大电流1800mA
	20000, 2250, // 20V @ 2.25A	//电小二移动电源PD 45W取电正常 PID最大电流1800mA
	15000, 3000, // 15V @ 3.0A	//22.5W 小米PD电源 65W取电正常 100%占空比最大电流1500mA
	15000, 2000, // 15V @ 3.0A	//22.5W 小米PD电源 65W取电正常 100%占空比最大电流1500mA
	12000, 3000, // 12V @ 3.0A	//12V下加热到300度耗时33秒
    10000, 2000, // 10V @ 2A
     9000,  3000, // 9V @ 3A
     9000,  2000, // 9V @ 2A
#endif
     5000,  100,  // 5V @ whatever
};
const uint8_t USB_PD_Desired_Levels_Len = 7;

// 用于存储PD挡位
uint16_t USB_PD_PDO_Levels[NUM_PDO_LEVELS] = {0};

#endif
