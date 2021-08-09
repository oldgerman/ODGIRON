 /*
 * cppports.cpp
 *
 *  Created on: Jun 10, 2021
 *      Author: OldGerman
 */
/**
 * Notice
 * 		BOOTLDR工程的.ld文件配置为对FLASH仅使用前48KB
 * 		APP工程的.ld文件配置为对FLASH仅使用从0x08010000开始的剩余空间192KB，
 * 		从0x0800C0000开始的1个Sector为16KB当成EEPROM使用，用于存放systemSettings
 * 		若想自己做用户APP工程，需要根据BOOTLDR工程的配置，手动更改 .ld文件 (链接文件)的
 * 		FLASH起始地址，和VECT_TAB_OFFSET
 * Acknowledgement：
 * 	1. 正点原子 STM32F1 开发指南（HAL 库版）：第五十二章 串口 IAP 实验
 * 	2. STM32F4系列的app和boot相互跳转 https://blog.csdn.net/gexueyuan/article/details/39694071
 * 	3. 创建 STM32F103 DFU 引导加载程序 https://mcus.tistory.com/69
 * 	   使用 STM32F103 DFU 引导加载程序更新固件 https://www.os4all.com/70?category=684960
 * 	4. STM32CUBEIDE 将变量定义到指定FLASH地址，以F4为例 https://www.cnblogs.com/svchao/p/13955517.html
 * 	   STM32CubeIDE 将变量定义到指定RAM地址——NOLOAD属性的重要性 https://bbs.huaweicloud.com/blogs/237238
 */
#include "cppports.h"
#include "Buttons.hpp"
#include "usb_device.h"
#include "usbd_dfu_if.h"
ButtonState AutoValue::buttonState = BUTTON_NONE;

uint16_t waitTimeUSBRstVal = 1000;
uint16_t timeStepGUIDelay = 30;
AutoValue waitTimeUSBRst(&waitTimeUSBRstVal, 4, 1000, 0, timeStepGUIDelay, timeStepGUIDelay, false);

uint16_t screenBrightnessVal = 0;
AutoValue screenBrightness(&screenBrightnessVal, 3, 100, 0, 5, 10, false);
const uint8_t startLogo[] = { 0x78, 0x1E, 0xE0, 0xF3, 0x01, 0x78, 0x00, 0xE0,
		0x83, 0xCF, 0x07, 0x80, 0xE7, 0x01, 0x0E, 0x78, 0x3C, 0x3C, 0xE0, 0xC3,
		0x03, 0x3C, 0x00, 0xE0, 0x83, 0x0F, 0x0F, 0xC0, 0xC3, 0x03, 0x0E, 0x60,
		0x1E, 0x78, 0xE0, 0x83, 0x07, 0x1E, 0x40, 0xE0, 0x83, 0x0F, 0x1E, 0xE0,
		0x81, 0x07, 0x1E, 0x40, 0x1E, 0x78, 0xE0, 0x83, 0x07, 0x1E, 0xF0, 0xE0,
		0x83, 0x0F, 0x1E, 0xE0, 0x81, 0x07, 0x1E, 0x40, 0x1F, 0xF8, 0xE0, 0x83,
		0x07, 0x1F, 0xF0, 0xE0, 0x83, 0x0F, 0x1E, 0xF0, 0x81, 0x0F, 0x3E, 0x00,
		0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0xF0, 0xE0, 0x83, 0x0F, 0x1E, 0xF0,
		0x81, 0x0F, 0x7C, 0x00, 0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0x60, 0xE0,
		0x83, 0x0F, 0x1E, 0xF0, 0x81, 0x0F, 0x78, 0x00, 0x1F, 0xF8, 0xE0, 0x83,
		0x07, 0x1F, 0x00, 0xE0, 0x83, 0x0F, 0x1E, 0xF0, 0x81, 0x0F, 0xF8, 0x00,
		0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0x00, 0xE0, 0x83, 0x0F, 0x0F, 0xF0,
		0x81, 0x0F, 0xF0, 0x00, 0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0x00, 0xE0,
		0x83, 0x8F, 0x07, 0xF0, 0x81, 0x0F, 0xF0, 0x01, 0x1F, 0xF8, 0xE0, 0x83,
		0x07, 0x1F, 0xFE, 0xE0, 0x83, 0xCF, 0x03, 0xF0, 0x81, 0x0F, 0xE0, 0x01,
		0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0xF0, 0xE0, 0x83, 0x0F, 0x0F, 0xF0,
		0x81, 0x0F, 0xE0, 0x03, 0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0xF0, 0xE0,
		0x83, 0x0F, 0x0F, 0xF0, 0x81, 0x0F, 0xC0, 0x03, 0x1F, 0xF8, 0xE0, 0x83,
		0x07, 0x1F, 0xF0, 0xE0, 0x83, 0x0F, 0x0F, 0xF0, 0x81, 0x0F, 0xC0, 0x07,
		0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0xF0, 0xE0, 0x83, 0x0F, 0x1F, 0xF0,
		0x81, 0x0F, 0x80, 0x0F, 0x1F, 0xF8, 0xE0, 0x83, 0x07, 0x1F, 0xF0, 0xE0,
		0x83, 0x0F, 0x1F, 0xF0, 0x81, 0x0F, 0x00, 0x0F, 0x1E, 0x78, 0xE0, 0x83,
		0x07, 0x1E, 0xF0, 0xE0, 0x83, 0x0F, 0x1F, 0xE0, 0x81, 0x07, 0x02, 0x1F,
		0x1E, 0x78, 0xE0, 0x83, 0x07, 0x1E, 0xF0, 0xE0, 0x83, 0x0F, 0x1F, 0xE0,
		0x81, 0x07, 0x02, 0x1E, 0x3C, 0x3C, 0xE0, 0xC3, 0x03, 0x3C, 0x78, 0xE0,
		0x83, 0x0F, 0x1E, 0xC0, 0xC3, 0x03, 0x06, 0x3E, 0x78, 0x1E, 0xE0, 0xF3,
		0x01, 0x78, 0x3C, 0xE0, 0x83, 0x0F, 0x7C, 0x80, 0xE7, 0x01, 0x3E, 0x7C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x10, 0xF8, 0x47, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0x49, 0xF4, 0x1F, 0x04, 0x04, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0xF8, 0xE7, 0x8E,
		0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0x49, 0x54, 0x95, 0x3C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x29, 0x49, 0xE4, 0x8F, 0x04, 0x04, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0xF9, 0x07, 0x80,
		0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x7D, 0x49, 0xF4, 0x9F, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x45, 0x45, 0x24, 0x89, 0x04, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBD, 0x45, 0x96, 0xD1,
		0x7F, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setContrast(uint16_t val) {
	u8g2.setContrast(map(*screenBrightness.val, 0, 100, 0, 255));
}

/**
 * 熄屏函数
 */
void shutScreen() {
	for (;;) {
		screenBrightness--;
		setContrast(*screenBrightness.val);
		if (*screenBrightness.val == screenBrightness.lower)
			break;
		GUIDelay();
		resetWatchdog();
	}
}

/**
 * 亮屏函数
 */
void brightScreen() {
	for (;;) {
		screenBrightness++;
		setContrast(*screenBrightness.val);
		waitTimeUSBRst--;
		if (*screenBrightness.val == screenBrightness.upper)
			break;
		GUIDelay();
		resetWatchdog();
	}
}

void GUIDelay() {
	// Called in all UI looping tasks,
	// This limits the re-draw rate to the LCD and also lets the DMA run
	// As the gui task can very easily fill this bus with transactions, which will
	// prevent the movement detection from running

	HAL_Delay(timeStepGUIDelay);

}

/**
 * C 从__DATE__宏获取编译日期函数
 * https://www.cnblogs.com/oyjj/archive/2011/04/24/2132874.html
 * 依赖：string.h
 */
typedef struct tagXDate {
	int year;
	int month;
	int day;
} XDate;

XDate compileXDate;	//编译日期决定版本号后3组XX数字："v1.0.XX.XX.XX"

bool GetCompileDate(XDate *date) {
	bool succeed = true;
	char complieDate[] = { __DATE__ };	//"Jul 06 2021"
	//字符串长度，可使用strlen()函数直接求出，切记，在使用strlen()求出字符串长度时，勿忘+1

	/**
	 strtok、strtok_s、strtok_r 字符串分割函数
	 https://blog.csdn.net/hustfoxy/article/details/23473805
	 */
	char *ptr;
	ptr = strtok(complieDate, " ");
	char *month = ptr;
	ptr = strtok(nullptr, " ");
	char *day = ptr;
	ptr = strtok(nullptr, " ");
	char *yearNoIntercept = ptr;	//未截取的年分：4位
	char year[3] = { 0 };					//储存截取年份的后2位
	/*
	 * C语言截取从某位置开始指定长度子字符串方法
	 * https://blog.csdn.net/zmhawk/article/details/44600075
	 */
	strncpy(year, yearNoIntercept + 2, 2);	//截取年后两位
	ptr = strtok(nullptr, " ");
	date->day = atoi(day);	//atoi()函数：将字符串转换成int(整数)
	if (date->day == 0)
		succeed = false;
	date->year = atoi(year);	//atoi()函数：将字符串转换成int(整数)
	if (date->year == 0)
		succeed = false;
	//依次判断月份
	const char months[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
			"Aug", "Sep", "Oct", "Nov", "Dec" };
	date->month = 0;
	for (int i = 0; i < 12; i++) {
		if (strcmp(month, months[i]) == 0) {
			date->month = i + 1;
			break;
		}
	}
	if (date->month == 0)
		succeed = false;
	return succeed;
}

void columsHome_ShowVerInfo() {
	bool enterDFU = true;	//标记
	if(!systemSettings.ResetForceDFU)	//判断是否是从用户APP辅助功能重启到DFU的
	{
		//第一页
		u8g2.drawXBM(0, 0, 128, 32, startLogo);
		u8g2.setFont(u8g2_font_IPAandRUSLCD_tf); //7pixel字体;

		char buf[18] { 0 };	//"v1.0.21.06.12"; 固定13个可打印字符
		if (GetCompileDate(&compileXDate)) {
			snprintf(buf, sizeof(buf), "(B)v1.0.%02d.%02d.%02d", compileXDate.year,
					compileXDate.month, compileXDate.day);
		} else {
			snprintf(buf, sizeof(buf), "(B)v1.0.XX.XX.XX");
		}
		uint8_t x = OLED_WIDTH - strlen(buf) * 5/*5=字体宽度*/- 2/*计算失误的偏差*/;	//版本号右对齐
		u8g2.drawStr(x, 24, buf);
		u8g2.sendBuffer();

		USB_Status_Init();
		brightScreen();	//先绘制，后亮屏

		uint16_t timeWaitingStartPage = HAL_GetTick();

		for (;;) {
			buttons = getButtonState();
			resetWatchdog();
			GUIDelay();
			if (buttons == BUTTON_OK_LONG)	//开机在DFU里等待多少时间进入APP，用户可配置
				break;	//长按中键进DFU页面
			if (HAL_GetTick() - timeWaitingStartPage > *waitTimeUSBRst.val) {
				enterDFU = false;
				break;
			}
			//超时退出
		}
	}


	//第二页：DFU升级页面
	if (enterDFU) {
		u8g2.clearBuffer();
		u8g2.sendBuffer();
		//u8g2.setDrawColor(1);
		//u8g2.drawBox(0, 0, 128, 32);
		u8g2.setDrawColor(1);
		//u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
		u8g2.setFont(u8g2_font_8x13B_tr);
		uint8_t y = 0;
		const char buf[15] = "USB DFU IAP";
		uint8_t x = (OLED_WIDTH - strlen(buf) * 8) / 2;	//版本号右对齐

		u8g2.drawStr(x, y - 1, buf);//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
		//				   "OK       Cancel"
		//若未实现
		u8g2.setFont(u8g2_font_IPAandRUSLCD_tf); //7pixel字体;
		y += 24;
		u8g2.drawStr(1, y, "<Press key A/B to RST>");
		uint8_t old_usb_status = 0;
		uint8_t cntBuzzerTime = 0;
		int8_t ok = -1;
		y -= 11;
		u8g2.drawStr(1, y, "[Status] Waiting...");
		u8g2.sendBuffer();

#if 1
		if(systemSettings.ResetForceDFU){
			systemSettings.ResetForceDFU = 0;
			saveSettings();	//为什么不能存储？？？
			brightScreen();	//先绘制，后亮屏
		}
#endif

		MX_USB_DEVICE_Init();	//启动USB-DFU
		for (;;) {
			resetWatchdog();
			ButtonState buttons = getButtonState();
			switch (buttons) {
			case BUTTON_B_SHORT:
				ok = 1;
				break;
			case BUTTON_F_SHORT:
				ok = 0;
				break;
			default:
				break;
			}

			cntBuzzerTime++;
			if (cntBuzzerTime == 3)
				cntBuzzerTime = cntBuzzerTime % 4;

			if (hUsbDeviceFS.dev_state != old_usb_status) {
				u8g2.setDrawColor(0);
				u8g2.drawBox(0, 13, 128, 8);
				u8g2.setDrawColor(1);
				if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
					//printf("连接成功\r\n");
					u8g2.drawStr(1, y, "[Status] USB Connected");
				else if (hUsbDeviceFS.dev_state == USBD_STATE_SUSPENDED)
					//printf("断开成功\r\n");
					u8g2.drawStr(1, y, "[Status] USB Suspended");
				else {
					u8g2.drawStr(1, y, "[Status] Waiting...");
				}
				old_usb_status = hUsbDeviceFS.dev_state;
				u8g2.sendBuffer();
			}

			if (ok != -1)
				break;
			GUIDelay();
		}


		//waitingToChooseOneFromTwo();
		//*打印重启提示
		shutScreen();
		NVIC_SystemReset();	//重启以退出DFU
	}
}

void (*Jump_To_Application)();
uint32_t JumpAddress;

void doGUIWork() {
	//若未执行此句，则SolderingTemp=10 TempChangeLongStep=0 TempChangeShortStep=0
	//resetSettings();	//手动将宏定义的值赋值给变量
	restoreSettings();

	u8g2_begin();
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//无法打印填充空格
	u8g2.setDrawColor(1);
	u8g2.setFontPosTop();
	u8g2.setFontDirection(0);
	u8g2.clearBuffer();
	u8g2.setDisplayRotation(U8G2_R0);

#if 0
	screenBrightness.upper = 100;
	u8g2.drawXBM(0, 0, 128, 32, startLogo);
	u8g2.sendBuffer();
	brightScreen();
	//u8g2.setContrast(128);
	waitingToChooseOneFromTwo();
#else
	//从Flash恢复屏幕亮度设置
	screenBrightness.upper = systemSettings.ScreenBrightness;
	columsHome_ShowVerInfo();	//显示DFU开机页
#endif
	checkAPPJumper();
}

void checkAPPJumper() {
	/**
	 * 判断USBD_DFU_APP_DEFAULT_ADD地址储存的是否是SP堆栈指针（Initial Main SP）
	 * （M3中断向量表的第一个4byte存放的是堆栈指针）
	 * 通过判断那里存放的值是不是正确的堆栈指针的值，来确定flash里面有没有用户程序。
	 * stm32的ram空间地址范围是0x20000000~0x2001ffff，共128K
	 * (42x和43x的ram空间更大，但是我们使用前128K进行判断足够了)
	 *
	 * Notice：
	 * 	USBD_DFU_APP_DEFAULT_ADD 的值不随CubeMX的修改后执行自动生成代码而变化，需要手动修改
	 */

	if (((*(__IO uint32_t*) USBD_DFU_APP_DEFAULT_ADD) & 0x2FFE0000) == 0x20000000)
	{
		/**
		 * USBD_DFU_APP_DEFAULT_ADD保存的是用户程序的首地址，
		 * 所以*(__IO uint32_t*) (USBD_DFU_APP_DEFAULT_ADD + 4)
		 * 就是用户程序中(第二个中断向量表中的)复位向量的地址
		 */
		JumpAddress = *(__IO uint32_t*) (USBD_DFU_APP_DEFAULT_ADD + 4);
		Jump_To_Application = (pFunction) JumpAddress;//将函数指针Jump_To_Application指向复位向量的地址
		__set_MSP(*(__IO uint32_t*) USBD_DFU_APP_DEFAULT_ADD);//设置堆栈指针sp，指向用户代码的首地址
        for(int i = 0; i < 8; i++)
        {
                //NVIC->ICER[i] = 0xFFFFFFFF; /* 关闭中断*/
                //NVIC->ICPR[i] = 0xFFFFFFFF; /* 清除中断标志位 */
        }
		Jump_To_Application();						//跳转到用户程序，把函数指针赋值给pc指针
	}
	//若没有跳转用户APP则打开USB-DFU
	//MX_USB_DEVICE_Init();
	NVIC_SystemReset();	//没有固件，再次重启
}
