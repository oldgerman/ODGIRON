/*
 * Model_Config.h
 *
 *  Created on: 25 Jul 2020
 *      Author: Ralim
 *      Modify: OldGerman
 */

#ifndef BSP_MINIWARE_MODEL_CONFIG_H_
#define BSP_MINIWARE_MODEL_CONFIG_H_


#define MODEL_TS80P	//需要自己加
#define ODGIRON		//德国佬的烙铁
/*
 * Lookup for mapping features <-> Models
 */

#if defined(MODEL_TS100) + defined(MODEL_TS80) + defined(MODEL_TS80P) > 1
#error "Multiple models defined!"
#elif defined(MODEL_TS100) + defined(MODEL_TS80) + defined(MODEL_TS80P) == 0
#error "No model defined!"
#endif

#ifdef MODEL_TS100
#define POW_DC
#define ACCEL_MMA
#define ACCEL_LIS
#define TEMP_TMP36
#define BATTFILTERDEPTH 32
#endif

#ifdef MODEL_TS80
#define ACCEL_LIS
#define POW_QC
#define TEMP_TMP36
#define LIS_ORI_FLIP
#define OLED_FLIP
#define BATTFILTERDEPTH 8
#endif

#if defined(MODEL_TS80P) + defined(ODGIRON) > 1
#define POW_PD
#define ACCEL_LIS
//#define TEMP_TMP36
#define I2C_SOFT
#define TEMP_MCP9700AT
//#define TEMP_STM32
#define BATTFILTERDEPTH 8
#define LIS_ORI_FLIP		//加速度计旋转检测
#define OLED_FLIP   		//oled旋转
#endif

#if MODEL_TS80P_ORG
#define ACCEL_LIS
#define ACCEL_MSA
#define POW_PD
#define POW_QC	    //QC
#define TEMP_NTC	//NTC测温
#define I2C_SOFT	//软件I2C？是不是一共有两路i2c？对！因为F103T8U6只有一路I2C1，所以用I2CBB这个类去搞什么信号量管理FUSB302的I2C
#define LIS_ORI_FLIP		//加速度计旋转检测
#define OLED_FLIP   		//oled旋转
#define BATTFILTERDEPTH 8
#endif

#endif /* BSP_MINIWARE_MODEL_CONFIG_H_ */
