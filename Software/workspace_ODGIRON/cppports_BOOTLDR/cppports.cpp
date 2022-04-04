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


void columsHome_ShowVerInfo() {
	//bool firstIn = true;	//标记
	bool enterDFU = true;	//标记
	USB_Status_Init();		//复位USB外设

	//uint16_t timeWaitingStartPage = HAL_GetTick();
	if (!getButtonOK())	//长按中键进DFU页面
		enterDFU = false;

	//第二页：DFU升级页面
	if (enterDFU) {

		drawLogoAndVersion('B');
		u8g2.sendBuffer();
		brightScreen();	//先绘制，后亮屏

		uint16_t timeWaitingStartPage = HAL_GetTick();
		for (;;) {
			if (HAL_GetTick() - timeWaitingStartPage > 666)
				break;
			resetWatchdog();
			GUIDelay();
		}

		u8g2.clearBuffer();
		//u8g2.sendBuffer();

		u8g2.setDrawColor(1);
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
		brightScreen();	//先绘制，后亮屏


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
	//restoreSettings();

	u8g2_begin();
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//无法打印填充空格
	u8g2.setDrawColor(1);
	u8g2.setFontPosTop();
	u8g2.setFontDirection(0);
	u8g2.setDisplayRotation(U8G2_R0);
	u8g2.clearBuffer();	//清屏

	screenBrightness.upper = 100;
	columsHome_ShowVerInfo();	//显示DFU开机页

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
