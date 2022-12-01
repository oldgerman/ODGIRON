/*
 * GUIThread.cpp
 *
 *  Created on: 19 Aug 2019
 *      Author: ralim
 *      Modify: OldGerman
 */
extern "C" {
#include "FreeRTOSConfig.h"
}
#include "Threads.hpp"
#include "configuration.h"
#include "LIS3DH.hpp"
#include "Settings.h"
#include <TipThermoModel.hpp>
#include "Translation.h"
#include "cmsis_os.h"
#include "main.h"
#include "stdlib.h"
#include "string.h"
#include "unit.h"
#include <gui.hpp>
#include <history.hpp>
#include <power.hpp>
#include "IRQ.h"	//提供ADC_Injected_Callback_Mark标记
#ifdef POW_PD
#include "policy_engine.h"
#include "int_n.h"
#include "protocol_rx.h"
#include "protocol_tx.h"
#endif

#include "font24x32numbers.h"
#include <deque>
#include <vector>
#include "Arduino.h"
#include "u8g2_simsun_9_fntodgironchinese.h"
#include "math.h"
#include "Page.hpp"
/*
 * 二分法解一元三次方程在区间(a, b)内的根
 * 输入对象:
 * 	a_0+a_1 \cdot (x-X) +  a_2 \cdot (x-X)^2+ a_3\cdot (x - X)^3 = y
*/
double solveCubicEquations(double y/*已知y求x*/,
		double *aArray = systemSettings.cala, double X = systemSettings.calX) {
	double a0 = *(aArray);
	double a1 = *(aArray + 1);
	double a2 = *(aArray + 2);
	double a3 = *(aArray + 3);

	//usb_printf("a0 = %d, a1 = %d, a2 = %d, a3 = %d, X = %d\r\n", (int)a0, (int)a1,(int)a2,(int)a3, (int)X);

	/*区间(a, b)
		注意精确范围，若太大则可能出现被其它根抢先结束运算
		例如:
		当y = 112
		double aArray[] = {231.727493286132, 1.041208863258, -0.000256661367, 0.000004276292};
		double X = 206.600006103515;
		区间(-1100 - X,1500 - X) 解为543
		区间(-100 - X, 500 - X)  解为114，这个才是要的值
	*/
	double a = -100 - X;
	double b = 500 - X;

	double c;	//	c == x - X
	double f1, f2, f3;
	double x;

	//double limitVal = 1e-6; //原作者默认值，太精
	double limitVal = 1; //如果计算次数太深，则尝试增加这个值，精度够用就好
	do {
		f1 = a3 * pow(a, 3) + a2 * pow(a, 2) + a1 * a + (a0 - y); //  得出左端项的值f1
		f2 = a3 * pow(b, 3) + a2 * pow(b, 2) + a1 * b + (a0 - y); //  得出右端项的值f2
		if (f1 * f2 < 0)  //  该条件成立说明该区间内有解
			{
			c = (a + b) / 2;  //  二分法取中间值
			f3 = a3 * pow(c, 3) + a2 * pow(c, 2) + a1 * c + (a0 - y);
			if (f1 * f3 < 0)  //  说明左端项和中间项有解
			{
				b = c;
			} else  //  说明右端项和中间项有解
			{
				a = c;
			}
		} else {
			return y; //该方程在此区间内无解, 直接返回y!
		}
	} while (fabs(f3) > limitVal);

	x = c + X;
	return x;
}

void u8g2Prepare(void) {
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//无法打印填充空格
	u8g2.setFontRefHeightText();
	u8g2.setDrawColor(1);
	u8g2.setFontPosTop();
	u8g2.setFontDirection(0);
}

//uint8_t idleScreenBGF[sizeof(idleScreenBG)];

// File local variables
extern uint32_t currentTempTargetDegC;
extern TickType_t lastMovementTime;
extern osThreadId GUITaskHandle;
extern osThreadId MOVTaskHandle;
extern osThreadId PIDTaskHandle;
extern osThreadId POWTaskHandle;

static bool shouldBeSleeping(bool inAutoStart = false);
static bool shouldShutdown();
void showWarnings();
uint16_t getRefTemperatureX10();

#define BUTTON_INACTIVITY_TIME   (60 * configTICK_RATE_HZ)
#define MOVEMENT_INACTIVITY_TIME (60 * configTICK_RATE_HZ)
static TickType_t lastHallEffectSleepStart = 0;

TipState tipState;
uint16_t DegCTip;
uint16_t degCTipCurvePage;

uint16_t screenBrightnessVal = 0;
AutoValue screenBrightness(&screenBrightnessVal, 3, 100, 0, 10, 10, false);
uint16_t tipDisconnectedThres; //开路阈值温度，每次上电，菜单修改温度负偏移、校准Calibration值后重新计算，此值也影响可调温度上限
uint16_t tipTempMaxAdjust;		   //焊接模式可调温度上限
AutoValue solderingTemp(&systemSettings.SolderingTemp, 3, 450, 0, 1, 10);
//uint8_t screenBrightness = systemSettings.screenBrightness;

void printTipInCNumber(uint16_t number, uint8_t x,
		bool showLeadingeZero = false);
void drawRightParameters(uint8_t Xcol = 109);	//Xcol为128时刚好不显示
void drawHeatSymbol(uint8_t state, int8_t Xcol = 0);
void drawLeftParameters(int8_t Xcol = 0);	//Xcol为-11时刚好不显示
/**
 * 熄屏函数
 * 阻塞按钮动作
 */
void shutScreen() {
	for (;;) {
		screenBrightness--;
		setContrast(*screenBrightness.val);
		if (*screenBrightness.val == screenBrightness.lower)
			break;
		GUIDelay();
	}
}

/**
 * 亮屏函数
 * 阻塞按钮动作
 */
void brightScreen() {
	for (;;) {
		screenBrightness++;
		setContrast(*screenBrightness.val);
		u8g2.setPowerSave(0);
		screenBrightness++;
		if (*screenBrightness.val == screenBrightness.upper)
			break;
		//GUIDelay();
		osDelay(100);
	}
}

#if 0
/**
 * 这个所谓的校准过程，仅仅是消除上拉电阻R26产生的偏差，不是用于校准烙铁本身的温度。。。
 * 所以，这个上拉其实另有用意，是不是加速运放退出饱和区啊？？？
 */
static void setTipOffset() {

	// 注意这里置0，那么接下来的TipThermoModel::convertTipRawADCTouV()减去的校准偏差就是0
	systemSettings.CalibrationOffset = 0;

	// 如果热电偶位于尖端的末端，并且手柄位于是衡温，则输出应为零，因为没有温度差。
	// If the thermo-couple at the end of the tip, and the handle are at
	// equilibrium, then the output should be zero, as there is no temperature
	// differential.

	while (systemSettings.CalibrationOffset == 0) {
		uint32_t offset = 0;

		// 执行16次【8次过采样】并求和
		for (uint8_t i = 0; i < 16; i++) {
			offset += getTipRawTemp(1);
			// cycle through the filter a fair bit to ensure we're stable.
			// 循环通过过滤器以确保我们稳定。
			buttons = getButtonState();
			osDelay(100);
		}
		//将8次过采样16次尖端温度的求和再求均值传入TipThermoModel::convertTipRawADCTouV()得到校准偏差为0时，
		//烙铁正负极由于上拉电阻产生的分压，这个值用作新的校准偏差值
		systemSettings.CalibrationOffset = TipThermoModel::convertTipRawADCTouV(
				offset / 16);
	}

	//osDelay(1200);
}
#endif

void GUIDelay() {
	// Called in all UI looping tasks,
	// This limits the re-draw rate to the LCD and also lets the DMA run
	// As the gui task can very easily fill this bus with transactions, which will
	// prevent the movement detection from running

	//osDelay(50);
	vTaskDelay(5 * TICKS_10MS);
}

void oledPrintTaskUsage(uint8_t xoffset, uint16_t stackSize,
		osThreadId TaskHandle) {
	static uint8_t indexColums = 0;
	char buf[23] = { 0 };
	uint16_t stackFreeSize = uxTaskGetStackHighWaterMark(TaskHandle);
	uint16_t used = stackSize - stackFreeSize;
	sprintf(buf, "%3d/%-3d  %d%%", used, stackSize, used * 100 / stackSize);
	u8g2.drawStr(xoffset - 36, indexColums * 8, buf);

	indexColums = (indexColums + 1) % 4;
	osDelay(30);
}

void showDebugMenu(void) {
	/*setup*/
	uint8_t screen = 0;
	ButtonState b;
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);
	u8g2.setFontRefHeightText();
	const uint8_t Xoffset = 94;
	const uint8_t numPlaces = 6;
	//bool calibrationDone = false;
	//setTipOffset();
	/*loop*/
	for (;;) {
		u8g2.clearBuffer();
		//u8g2.sendBuffer();
		for (int i = 0; i < 4; i++) {
			//osDelay(10);
			u8g2.drawStr(0, i * 8, DebugMenu[screen * 4 + i]);
		}

		//首页菜单功能轮询
		switch (screen) {
		case 0:
			// Just prints version、date、author

			break;
		case 1:
			// High water mark for GUI
			oledPrintTaskUsage(Xoffset, GUITask_stacksize, GUITaskHandle);
			oledPrintTaskUsage(Xoffset, MOVTask_stacksize, MOVTaskHandle);
			oledPrintTaskUsage(Xoffset, PIDTask_stacksize, PIDTaskHandle);
			oledPrintTaskUsage(Xoffset, POWTask_stacksize, POWTaskHandle);
			break;

		case 2:
			oledPrintTaskUsage(Xoffset, InterruptHandler::TaskStackSize,
					InterruptHandler::TaskHandle);
			oledPrintTaskUsage(Xoffset, PolicyEngine::TaskStackSize,
					PolicyEngine::TaskHandle);
			oledPrintTaskUsage(Xoffset, ProtocolReceive::TaskStackSize,
					ProtocolReceive::TaskHandle);
			oledPrintTaskUsage(Xoffset, ProtocolTransmit::TaskStackSize,
					ProtocolTransmit::TaskHandle);
			break;

		case 3:
			// system up time stamp
			drawNumber(Xoffset, 0, xTaskGetTickCount() / 100, numPlaces);
			drawNumber(Xoffset, 8, DetectedAccelerometerVersion, numPlaces);
			drawNumber(Xoffset, 16, lastMovementTime / 100, numPlaces);
			drawNumber(Xoffset, 24, lastButtonTime / 100, numPlaces);
			break;

		case 4:

		{
			// Raw Tip
			static uint32_t mVTip = 0;
			drawNumber(Xoffset, 0, mVTip, numPlaces);
			// Temp in C
			drawNumber(Xoffset, 8, TipThermoModel::getTipInC(), numPlaces);
			// Handle Temp
			drawNumber(Xoffset, 16, getHandleTemperature(true), numPlaces);
			// Max deg C limit
			drawNumber(Xoffset, 24, TipThermoModel::getTipMaxInC(), numPlaces);

			mVTip = TipThermoModel::convertTipRawADCTouV(getTipRawTemp(0))
					/ 1000;
		}
			break;

		case 5:

			// Power negotiation status
			if (getIsPoweredByDCIN()) {
				drawNumber(Xoffset, 0, 0, numPlaces);
			} else {
				// We are not powered via DC, so want to display the appropriate state for PD or QC
				bool poweredbyPD = false;
#ifdef POW_PD
#if 1
				if (usb_pd_detect()) {//会报错：undefined reference to `PolicyEngine::pdNegotiationComplete'
					// We are PD capable
					if (PolicyEngine::pdHasNegotiated()) {
						// We are powered via PD
						poweredbyPD = true;
					}
				}
#endif
#endif
				if (poweredbyPD) {
					drawNumber(Xoffset, 0, 2, numPlaces);
				} else {

					drawNumber(Xoffset, 0, 1, numPlaces);
				}
			}

			// Voltage input
			drawNumber(Xoffset, 8, getInputVoltageX10(), numPlaces);
			drawNumber(Xoffset, 16, busAX1000, numPlaces);
			drawNumber(Xoffset, 24, systemSettings.CalibrationOffset,
					numPlaces);

			break;

		default:
			break;
		}

		u8g2.sendBuffer();

		b = getButtonState();

		if (b == BUTTON_B_SHORT) {
			if (screen == 0)
				screen = 6;
			screen--;
		} else if (b == BUTTON_F_SHORT) {
			screen++;
			screen = screen % 6;
		} else if (b == BUTTON_OK_SHORT)
			return;
		else
			;
		GUIDelay();
	}
}

static uint16_t min(uint16_t a, uint16_t b) {
	if (a > b)
		return b;
	else
		return a;
}

/**
 * 返回睡眠超时阈值时间
 */
static uint32_t getSleepTimeout() {
	if (systemSettings.Sensitivity && systemSettings.SleepTime) {
#if 0
		uint32_t sleepThres = 0;
		if (systemSettings.SleepTime < 6)
			sleepThres = systemSettings.SleepTime * 10 * 1000;
		else
			sleepThres = (systemSettings.SleepTime - 5) * 60 * 1000;

		return sleepThres;
#else
		return systemSettings.SleepTime * 1000;
#endif
	}
	return 0;
}
/**
 * 比较运动或按钮超时阈值，来决定返回是否睡眠
 * 超时并且未检测到运动和按钮动作返回true
 */
static bool shouldBeSleeping(bool inAutoStart) {
	// Return true if the iron should be in sleep mode
	if (systemSettings.Sensitivity && systemSettings.SleepTime) {
		if (inAutoStart) {
			// In auto start we are asleep until movement
			if (lastMovementTime == 0 && lastButtonTime == 0) {
				return true;
			}
		}
		if (lastMovementTime > 0 || lastButtonTime > 0) {
			if ((xTaskGetTickCount() - lastMovementTime) > getSleepTimeout()
				&& (xTaskGetTickCount() - lastButtonTime) > getSleepTimeout()) {
				return true;
			}
		}
	}

	return false;
}

static bool shouldShutdown() {
	//if (xTaskGetTickCount() > 5000 && (getInputVoltageX10() / 10) < 8)
	//	return true;

	if (systemSettings.ShutDownTime) { // only allow shutdown exit if time > 0
		if (((TickType_t) (xTaskGetTickCount() - lastMovementTime))
				> (TickType_t) ((systemSettings.SleepTime
						+ systemSettings.ShutDownTime) * 1000)) {
			return true;
		}
		if (lastHallEffectSleepStart) {
			if (((TickType_t) (xTaskGetTickCount() - lastHallEffectSleepStart))
					> (TickType_t) ((systemSettings.SleepTime
							+ systemSettings.ShutDownTime) * 1000)) {
				return true;
			}
		}
	}
	return false;
}

static void gui_solderingTempAdjust() {
	u8g2.clearBuffer();
	osDelay(50);
#define CARTOON_TEMP_ADJ 1
#if defined(CARTOON_TEMP_ADJ)
	const int steps = 6;	//动画帧数
	int16_t delta = (systemSettings.SolderingTemp - DegCTip) / steps;
	for(int i = 1; i < steps; i++)
	{
		u8g2.clearBuffer();
		printTipInCNumber((uint16_t)DegCTip + delta*i, 14);
		drawRightParameters(110 + i*3);
		drawLeftParameters(0 - i*2);
		//osDelay(10);	//不需要更流畅
		u8g2.sendBuffer();
	}
	u8g2.clearBuffer();
#endif

	uint32_t lastChange = xTaskGetTickCount();
	currentTempTargetDegC = 0;		//？？？
	uint32_t autoRepeatTimer = 0;
	uint8_t autoRepeatAcceleration = 0;


	for (;;) {
		//usb_printf("SolderingTemp=%d TempChangeLongStep=%d TempChangeShortStep=%d\r\n",systemSettings.SolderingTemp,systemSettings.TempChangeLongStep, systemSettings.TempChangeShortStep);
		buttons = getButtonState();
		if (buttons)
			lastChange = xTaskGetTickCount();
		switch (buttons) {
		case BUTTON_NONE:
			// stay
			break;
		case BUTTON_BOTH:
			// exit
			return;
			break;
		case BUTTON_B_LONG:
			if (xTaskGetTickCount() - autoRepeatTimer
					+ autoRepeatAcceleration> PRESS_ACCEL_INTERVAL_MAX) {
				if (systemSettings.ReverseButtonTempChangeEnabled) {
					systemSettings.SolderingTemp +=
							systemSettings.TempChangeLongStep;
				} else
					systemSettings.SolderingTemp -=
							systemSettings.TempChangeLongStep;

				autoRepeatTimer = xTaskGetTickCount();
				autoRepeatAcceleration += PRESS_ACCEL_STEP;
			}
			break;
			//short step是1℃
			//long step是10℃
		case BUTTON_B_SHORT:
			if (systemSettings.ReverseButtonTempChangeEnabled) {
				systemSettings.SolderingTemp +=
						systemSettings.TempChangeShortStep;
			} else
				systemSettings.SolderingTemp -=
						systemSettings.TempChangeShortStep;
			break;
		case BUTTON_F_LONG:
			if (xTaskGetTickCount() - autoRepeatTimer
					+ autoRepeatAcceleration> PRESS_ACCEL_INTERVAL_MAX) {
				if (systemSettings.ReverseButtonTempChangeEnabled) {
					systemSettings.SolderingTemp -=
							systemSettings.TempChangeLongStep;
				} else
					systemSettings.SolderingTemp +=
							systemSettings.TempChangeLongStep;
				autoRepeatTimer = xTaskGetTickCount();
				autoRepeatAcceleration += PRESS_ACCEL_STEP;
			}
			break;
		case BUTTON_F_SHORT:
			if (systemSettings.ReverseButtonTempChangeEnabled) {
				systemSettings.SolderingTemp -=
						systemSettings.TempChangeShortStep; // add 10
			} else
				systemSettings.SolderingTemp +=
						systemSettings.TempChangeShortStep; // add 10
			break;
		default:
			break;
		}
		if ((PRESS_ACCEL_INTERVAL_MAX - autoRepeatAcceleration)
				< PRESS_ACCEL_INTERVAL_MIN) {
			autoRepeatAcceleration = PRESS_ACCEL_INTERVAL_MAX
					- PRESS_ACCEL_INTERVAL_MIN;
		}
		// constrain between 10-450 C
#ifdef ENABLED_FAHRENHEIT_SUPPORT
    if (systemSettings.temperatureInF) {
      if (systemSettings.SolderingTemp > 850)
        systemSettings.SolderingTemp = 850;
      if (systemSettings.SolderingTemp < 60)
        systemSettings.SolderingTemp = 60;
    } else
#endif
		{
			if (systemSettings.SolderingTemp > tipTempMaxAdjust)
				systemSettings.SolderingTemp = tipTempMaxAdjust;
			if (systemSettings.SolderingTemp < 10)
				systemSettings.SolderingTemp = 10;
		}
		if ((xTaskGetTickCount() - lastChange > 2000)
				|| (buttons == BUTTON_OK_SHORT)) {
			break; // exit if user just doesn't press anything for a bit
		}

		printTipInCNumber(systemSettings.SolderingTemp, 14);

		if (u8g2.getRotation() != U8G2_R0) {
			u8g2.drawXBM(109, 0, 12, 32, symbolSubtract);
			u8g2.drawXBM(0, 0, 12, 32, symbolAdd);
		} else {
			u8g2.drawXBM(109, 0, 12, 32, symbolAdd);
			u8g2.drawXBM(0, 0, 12, 32, symbolSubtract);
		}

		u8g2.sendBuffer();
		GUIDelay();


	}

#if defined(CARTOON_TEMP_ADJ)
	u8g2.clearBuffer();
	osDelay(50);
	//经过调整后systemSettings.SolderingTemp和DegCTip与刚进本函数时的大小关系可能相反
	//因此再次采集一次DegCTip
	DegCTip = (uint16_t) TipThermoModel::getTipInC();
	delta = (systemSettings.SolderingTemp - DegCTip) / steps;
	for(int i = 1; i < steps; i++)
	{
		printTipInCNumber(systemSettings.SolderingTemp - delta*i, 14);
		drawRightParameters(128 - i*3);
		drawLeftParameters(-10 + i*2);
		//osDelay(10);	//不需要更流畅
		u8g2.sendBuffer();
	}
#endif
}

/**
 * @brief  打印开机logo
 * @note
 * @param  delayS 显示持续时间
 * @retval None
 */
void showLogo(uint8_t delayS) {
	//FreeRTOS在某一仍任务的setup()段写这样的代码会阻塞其他任务嘛？
	uint32_t ticks = xTaskGetTickCount();
	ticks += delayS * 1000; // 4 seconds from now
	while (xTaskGetTickCount() < ticks) {
		//log绘制函数
		if (showBootLogoIfavailable() == false)
			ticks = xTaskGetTickCount();

		//
		buttons = getButtonState();
		if (buttons == BUTTON_OK_LONG) //若按下则马上退出
			ticks = xTaskGetTickCount(); // make timeout now so we will exit
		GUIDelay();
	}
}

void printTipInCNumber(uint16_t number, uint8_t x, bool showLeadingeZero) {
	u8g2.setDrawColor(0);
	u8g2.drawBox(x, 0, 24 * 3, 32);	//覆盖之前字体
	u8g2.setDrawColor(1);

	uint8_t buffer[3] = { 0 };
	uint8_t exchange = 0;
	//uint8_t buffer[3] = { 9,8,5 };
	uint8_t *charPointer;

	for (int i = 0; i < 3; i++) {
		buffer[i] = number % 10;    //将各个位的数赋值于数组buffer[i] ；
		number /= 10;           	//现在数组buffer[i]记录的是各个位上的数的反序；

	}
	exchange = buffer[0];
	buffer[0] = buffer[2];
	buffer[2] = exchange;

	for (int i = 0; i < 3; i++) {
		if (i == 0 && buffer[0] == 0)
			;
		else if (i == 1 && buffer[0] + buffer[1] == 0)
			;
		//else if (i == 2 && buffer[0] + buffer[1] + buffer[2] == 0)
		//	;
		else {
			charPointer = (uint8_t*) numbers24x32 + buffer[i] * 24 * 4;
			u8g2.drawXBM(x + i * 24, 0, 24, 32, charPointer);
		}
	}
	u8g2.drawXBM(88, 0, 16, 32, symbolCelsius16x32);		//绘制℃符号
	//usb_printf("buffer: %d %d %d", 	buffer[0], buffer[1], buffer[2]);
}

// used to obtain the size of an array of any type
#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

enum GraphState {
	GRAPH_NONE, GRAPH_CLEAN, GRAPH_DRAW
};

class CurveGragh {
public:
	CurveGragh(uint16_t *data, uint16_t upperLimit, uint16_t lowerLimit)//记录总时间
	:
			_data(data), _upperLimit(upperLimit), _lowerLimit(lowerLimit) {
		_height = OLED_HEIGHT;
		_changeSubfied = true;

		_index = 0;
		_numDataOld = _numData;
		_widthMoreThan_numData = (_width >= _numData) ? 1 : 0;
	}

	uint8_t updateCurveGraph(GraphState graphState);
	void drawCurveGraph(GraphState graphState);
	static bool checkUpdateTime() {
		uint32_t previousState = xTaskGetTickCount();
		static uint32_t previousStateChange = xTaskGetTickCount();
		updateData = false;
		if ((previousState - previousStateChange)
				> (recordingTime / _numData)) {
			previousStateChange = previousState;
			updateData = true;
		}
		return updateData;
	}

	static uint8_t _x;
	static uint8_t _y;
	static uint8_t _width;
	static uint8_t _height;
	static uint16_t _numData;
	static uint16_t recordingTime;

private:
	static bool updateData;
	uint16_t *_data;
	uint8_t _numDataOld;
	uint8_t _subfied = 0;	//分栏个数
	bool _changeSubfied;	//标记分栏是否被修改.首次进入时为true
	uint16_t _upperLimit;
	uint16_t _lowerLimit;

	std::deque<float> _curveData;
	std::deque<float>::iterator _prevCurveDatafront;	//上一次更新数据时的第一个元素
	uint8_t _index;
	bool _widthMoreThan_numData;
};

uint8_t CurveGragh::_x = 0;
uint8_t CurveGragh::_y = 0;
uint8_t CurveGragh::_width = 88;
uint8_t CurveGragh::_height = OLED_HEIGHT;
uint16_t CurveGragh::_numData = 44;
uint16_t CurveGragh::recordingTime = 10000;	//ms 一页面波形更新约耗时11.28s
bool CurveGragh::updateData = false;

void CurveGragh::drawCurveGraph(GraphState graphState) {

	std::deque<float>::iterator itr = _curveData.begin();
	uint8_t i = 0;
	uint8_t factor = _width / _numData;
	int8_t Xoffset = 0;
	int16_t Xprev = -1, Xthis;
	uint16_t Yprev = 0, Ythis;
	if (graphState == GRAPH_DRAW) {
		u8g2.setDrawColor(1);
		Xoffset = 0;
	}
	else;

	if (_widthMoreThan_numData) {
		while (i < _numData) {
			i++;
			if ((itr + i) != _curveData.end()) {
				Xthis = _x + i * factor;
				Ythis = (_y + *(itr + i + Xoffset));
				if ((Xprev != -1) && graphState != GRAPH_NONE)	//&& (Yprev != 32))
						u8g2.drawLine(Xprev - factor, Yprev, Xthis - factor, Ythis);
				Xprev = Xthis;
				Yprev = Ythis;
				//usb_printf("Xprev = %d, Yprev = %d, Xthis = %d, Ythis = %d\r\n", Xprev, Yprev, Xthis, Ythis);
			} else
				break;
		}

	} else {	//样本数大于X格点数，取多个样本平均值映射到X格点
		;
	}
}

uint8_t CurveGragh::updateCurveGraph(GraphState graphState) {
	if (_changeSubfied) {
		_changeSubfied = false;
	}

	if (updateData) {
		//若小于长度则
		while (_index < _numData) {
			_index++;
			_curveData.push_back(31);	//无溢出现象
			//_curveData.push_back(0);	//有溢出现象
		}

		_prevCurveDatafront = _curveData.begin();
		_curveData.pop_front();

		_curveData.push_back(
				map((*_data != 0) ? *_data : 1, _lowerLimit, _upperLimit, 31,
						0));
		//drawCurveGraph(GRAPH_CLEAN);
		drawCurveGraph(graphState);
		return _curveData.back();
	}
	return 0;
}

// Draws a number at the current cursor location
// maximum places is 5
// maximum places is 5
void drawNumber(uint8_t x, uint8_t y, uint16_t number, uint8_t places,
		uint8_t padixPointOffset) {
	char buffer[7] = { 0 };

	sprintf(buffer, "%6d", number);
	uint8_t cntFirstNum = 0;
	uint8_t i = 0;
	while (i < 7) {
		if (buffer[i] != ' ') {
			cntFirstNum = i;
			break;
		}
		++i;
	}

	uint8_t cntNum = 6 - cntFirstNum;
	u8g2.drawStr(x, y, buffer + cntFirstNum - (places - cntNum));//这个函数不能传入负值
}

//uint16_t tipInC = 0;
uint16_t Watt = 0;
CurveGragh curveGraghTip(&degCTipCurvePage, 430, 0);
CurveGragh curveGraghWatt(&Watt, 47, 0);
uint8_t parityInHomePage_BUTTON_OK_SHORT = 0;//主页的单次OK短按的奇偶性标记，默认为0，即sleep模式，1为焊接模式

void waitForSwapPageTimeout() {
	uint32_t timeout = 400;
	timeout += xTaskGetTickCount();
	uint8_t steps = 0;
	u8g2.setDrawColor(0);
	for (;;) {
		steps += 2;
		u8g2.drawBox(0, -16 + steps, 108, 16);
		u8g2.drawBox(0, 32 - steps, 108, 16);
		u8g2.sendBuffer();
		buttons = getButtonState();
		if (xTaskGetTickCount() > timeout)
			break;
		//osDelay(20);	//不需要，更流畅
	}
	u8g2.setDrawColor(1);
}

uint16_t busAX1, busAX2, busAX3;

void drawRightParameters(uint8_t Xcol) {
#if 1
	//const uint8_t Xcol = 109;
	static uint32_t timeout_1 = xTaskGetTickCount();
	static uint8_t swap = 0;
	if (xTaskGetTickCount() - timeout_1 > 2000) {
		timeout_1 = xTaskGetTickCount();
		swap++;
	}
	swap %= 2;

	if (swap) {
		u8g2.drawStr(Xcol, 0, "VIN");
		drawNumber(Xcol, 8, getInputVoltageX10() / 10, 2);
		u8g2.drawPixel(Xcol + 12, 15);
		drawNumber(Xcol + 14, 8, getInputVoltageX10() % 10, 1);
		//osDelay(10);
		u8g2.drawStr(Xcol, 16, "LCP");

		if (!INADevicesFound) {
			busAX1000 = x10WattHistory.average() * 1000 / getInputVoltageX10();
		}
//2643
		busAX1 = busAX1000 / 1000;	//2
		busAX1000 = busAX1000 % 1000;	//643
		busAX2 = busAX1000 / 100;	//6
		busAX1000 = busAX1000 % 100;
		busAX3 = busAX1000 / 10;
		drawNumber(Xcol, 24, busAX1, 1);
		u8g2.drawPixel(Xcol + 6, 31);
		drawNumber(Xcol + 8, 24, busAX2, 1);
		drawNumber(Xcol + 14, 24, busAX3, 1);
	} else {
		u8g2.drawStr(Xcol, 0, "SET");
		drawNumber(Xcol, 8, systemSettings.SolderingTemp, 3);
		//osDelay(10);
		u8g2.drawStr(Xcol, 16, "REF");
#if 0
		drawNumber(Xcol, 24, getHandleTemperature(true) / 10, 2);
		u8g2.drawPixel(Xcol + 12, 31);
		drawNumber(Xcol + 14, 24, getHandleTemperature(true) % 100, 1);
#else
		//使用Ref偏移量
		drawNumber(Xcol, 24, getRefTemperatureX10() / 10, 2);
		u8g2.drawPixel(Xcol + 12, 31);
		drawNumber(Xcol + 14, 24, getRefTemperatureX10() % 100, 1);
#endif
	}

	//对于画太多操作，要加入osDelay来踹口气让其他任务调度，否则会出发最大延迟强制调度会打断UI绘制出现错位
	//加了就没有了出现错位了：
	osDelay(5);
#endif
}

uint16_t getRefTemperatureX10() {
	uint16_t tempOffsetRefVal = getHandleTemperature(true);
	AutoValue tempOffsetRef(&tempOffsetRefVal, 3, 999, 0, 0, 10, false);
	//												      ^ shortSteps为0，默认用longSteps
	//for (int i = 0; i < systemSettings.TempOffsetRef; i++)
	//	tempOffsetRef--;
	return *tempOffsetRef.val;
}

void showCurvePage() {
	waitForSwapPageTimeout();
	buttons = BUTTON_IDLE;
	//u8g2.clearBuffer();
	//u8g2.sendBuffer();
	for (;;) {
		if (CurveGragh::checkUpdateTime() || buttons == BUTTON_IDLE) {
			//taskENTER_CRITICAL();
			u8g2.clearBuffer();
			Watt = x10WattHistory.average() / 10;
			degCTipCurvePage = (uint16_t) TipThermoModel::getTipInC();
			if(systemSettings.CalibrationEnable) {
				degCTipCurvePage = calibrationHorner(degCTipCurvePage);
			}
			bool swap =
					(curveGraghTip.updateCurveGraph(GRAPH_DRAW)
							> curveGraghWatt.updateCurveGraph(GRAPH_DRAW)) ? 1 : 0;

			for (uint8_t i = 0; i < 29; i++) {
				u8g2.drawPixel(i * 3, 15);
			}
			for (uint8_t i = 0; i < 11; i++) {
				u8g2.drawPixel(42, i * 3);
			}
		//u8g2.drawLine(127,0,127,31);
		//u8g2.setDrawColor(1);
			drawRightParameters();

			const uint8_t XcolX1 = 88;
			if (swap) {
				u8g2.drawStr(XcolX1, 0, "PWR");
				drawNumber(XcolX1, 8, Watt, 3);
				osDelay(10);
				u8g2.drawStr(XcolX1, 16, "TIP");
				drawNumber(XcolX1, 24, degCTipCurvePage, 3);
			} else {
				u8g2.drawStr(XcolX1, 0, "TIP");
				drawNumber(XcolX1, 8, degCTipCurvePage, 3);
				osDelay(10);
				u8g2.drawStr(XcolX1, 16, "PWR");
				drawNumber(XcolX1, 24, Watt, 3);
			}
			osDelay(10);
			//taskEXIT_CRITICAL();
			u8g2.sendBuffer();
		}

		buttons = getButtonState();
		switch (buttons) {
		case BUTTON_NONE:
			break;
		case BUTTON_BOTH:
			break;

		case BUTTON_B_LONG:
			waitForSwapPageTimeout();
			return;	//返回主界面
		case BUTTON_F_SHORT:
			CurveGragh::recordingTime += 1000;
			break;
		case BUTTON_B_SHORT:
			if (CurveGragh::recordingTime >= 1000)
				CurveGragh::recordingTime -= 1000;
			break;

		case BUTTON_OK_SHORT:
			break;
		case BUTTON_OK_LONG:
			break;

		default:
			break;
		}

		if (!shouldBeSleeping(true)) {
			if(systemSettings.CalibrationEnable)
			currentTempTargetDegC = solveCubicEquations(systemSettings.SolderingTemp);	//还原设定温度
			else
				currentTempTargetDegC = systemSettings.SolderingTemp;	//还原设定温度
		}

		GUIDelay();
	}
}

/*
 * 检测是否连接烙铁
 * 返回true则连接
 */
void tipDetected() {
	//检测断开阈值 烙铁开路，并oled显示提示
	tipDisconnectedThres = TipThermoModel::getTipMaxInC() - 5; //- systemSettings.TempOffsetRef;
	tipTempMaxAdjust = tipDisconnectedThres - 10;		   //焊接模式可调温度上限

	static TipState tipStateLast;			//用于tipState从TIP_OPEN_CIRCUIT恢复至上次状态
	if(tipState != TIP_OPEN_CIRCUIT)
		tipStateLast = tipState;

	if (DegCTip > tipDisconnectedThres || heaterThermalRunaway) {	//开路检测
		tipState = TIP_OPEN_CIRCUIT;

		if(DegCTip > tipDisconnectedThres) {
			u8g2.setFont(u8g2_font_profont22_tr);
			u8g2.setFontRefHeightExtendedText();
			u8g2.drawStr(26, 15, "ACCESS");
			u8g2.drawStr(26, -2, "NO TIP");		//该字体高14pixel下对齐, 从下往上打印不会遮盖字体像素，列边缘间距3pixel
		}
		else{
			u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels中文字体
			u8g2.setFontRefHeightText();
			u8g2.drawUTF8(21, 1, "温度失控保护");
			u8g2.drawUTF8(21, 19, "长按左右退出");		//该字体高14pixel下对齐, 从下往上打印不会遮盖字体像素，列边缘间距3pixel
		}
		u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//还原8pixel字体
	}
	else {
		tipState = tipStateLast;
	}
}

void drawHeatSymbol(uint8_t state, int8_t Xcol) {
	// Draw symbol 14
	// Then draw over it, the bottom 5 pixels always stay. 8 pixels above that are
	// the levels masks the symbol nicely
	state /= 28; // 0-> 8 range
	uint8_t mmp = 2 + (8 - state);
	// Then we want to draw down (16-(5+state)
	u8g2.setDrawColor(1);
	u8g2.drawXBM(Xcol, 0, 11, 16, symbolHeating);
	u8g2.setDrawColor(0);
	u8g2.drawBox(Xcol, 0, 11, mmp);	//若drawBox的高度<0，则会导致后续绘制的显示错位+乱码
	u8g2.setDrawColor(1);
}

void setContrast(uint16_t val) {
	u8g2.setContrast(map(*screenBrightness.val, 0, 100, 0, 255));
}

template<class T, uint8_t SIZE>
struct historySizeAdj {
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




/**
 *卡尔曼滤波器
 *@param KFP *kfp 卡尔曼结构体参数
 *   float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float kalmanFilter(KFP* kfp, float input)
{
    //预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
    kfp->Now_P = kfp->LastP + (((float)(*kfp->Q))*2.5)/100;
    //卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
    kfp->Kg = kfp->Now_P / (kfp->Now_P + ((float)(*kfp->R))/100);
    //更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
    kfp->out = kfp->out + kfp->Kg * (input - kfp->out);//因为这一次的预测值就是上一次的输出值
    //更新协方差方程: 本次的系统协方差付给 kfp->LastP 威下一次运算准备。
    kfp->LastP = (1 - kfp->Kg) * kfp->Now_P;
    return kfp->out;
}

/**
 *调用卡尔曼滤波器 实践
 */
//给float型变量赋十六进制的值时（如0xffffffff)，都被认为是正数，当数值的二进制表示很长，则会发生截断
KFP KFP_Temp = {0.02, 0, 0, 0, &systemSettings.kalmanP, &systemSettings.kalmanQ};



void doGUITask() {
	//若未执行此句，则SolderingTemp=10 TempChangeLongStep=0 TempChangeShortStep=0
	resetSettings();	//没有Bootloader下debug时需要加上这句不然屏幕不亮，手动将宏定义的值赋值给变量
	restoreSettings();
	u8g2_begin();
	u8g2Prepare();
	u8g2.clearBuffer();
	u8g2.setDisplayRotation(
			(systemSettings.OrientationMode == ORIENTATION_LEFT_HAND) ?
					U8G2_R2 : U8G2_R0);


	uint8_t tempWarningState = 0;
	bool buttonLockout = false;

	getTipRawTemp(1); // reset filter

	//打印开机自检信息
	//showWarnings(5);

	//bool firstInSleep = false;

	//从Flash恢复开机工作模式
	if (systemSettings.AutoStartMode) {
		tipState = TIP_HEATING;
		//强制更新运动状态时间戳
		lastButtonTime = xTaskGetTickCount() - getSleepTimeout();
		lastMovementTime = xTaskGetTickCount() - getSleepTimeout();
	} else
		tipState = TIP_SLEEPING;

	//载入选择的语言
	ptrAutoValueLanguageVals->at(systemSettings.Language) = 1;

	//从Flash恢复屏幕亮度设置
	u8g2.setPowerSave(1);
	*screenBrightness.val = systemSettings.ScreenBrightness;
	screenBrightness.upper = *screenBrightness.val;
	setContrast(*screenBrightness.val);
	bool firstScreenBright = true; //亮屏过程阻塞开关检测的标记




	if(systemSettings.PowerOnShowLogo){
		//绘制开机logo
		drawLogoAndVersion('A');
		brightScreen();
		u8g2.sendBuffer();
		uint16_t timeWaitingStartPage = HAL_GetTick();
		for (;;) {
			if (HAL_GetTick() - timeWaitingStartPage > 666)
				break;
			resetWatchdog();
			GUIDelay();
		}
	}
	shutScreen();

	DegCTip = TipThermoModel::getTipInC();//第一次不进滤波器刷新一次

	for (;;) {
		if (CurveGragh::checkUpdateTime()) {
			curveGraghTip.updateCurveGraph(GRAPH_NONE);
			Watt = x10WattHistory.average() / 10;
			curveGraghWatt.updateCurveGraph(GRAPH_NONE);
		}

		//u8g2.clearBuffer();	//不要放在这，会导致切换Page没有动画
		if (firstScreenBright
				&& (*screenBrightness.val == screenBrightness.upper 	//从上电或熄屏唤醒首次达到最大亮度
						|| *screenBrightness.val == screenBrightness.lower))	//从熄屏最低亮度唤起
		{
			firstScreenBright = false;
			buttons = getButtonState();
		}
		if (!firstScreenBright) {
			buttons = getButtonState();
		}

		if (tempWarningState == 2)
			buttons = BUTTON_F_SHORT;
		if (buttons != BUTTON_NONE && buttonLockout)
			buttons = BUTTON_NONE;
		else
			buttonLockout = false;

		switch (buttons) {

		case BUTTON_NONE:
			// Do nothing
			break;
		case BUTTON_BOTH:
			// Not used yet
			// In multi-language this might be used to reset language on a long hold
			// or some such
			break;

		case BUTTON_B_LONG:
			showCurvePage();
			break;

		case BUTTON_F_LONG:
			showDebugMenu();
			//Not used yet
			break;

		case BUTTON_F_SHORT:
		case BUTTON_B_SHORT:
			gui_solderingTempAdjust(); // goto adjust temp mode
			buttonLockout = true;
			break;

		case BUTTON_OK_SHORT:
#if 0
			// 初始状态为休眠，按一次变为加热，再按一次为休眠，如此反复
			parityInHomePage_BUTTON_OK_SHORT = (parityInHomePage_BUTTON_OK_SHORT
					+ 1) % 2;
			(parityInHomePage_BUTTON_OK_SHORT == 0) ?
					(tipState = TIP_SLEEPING) :
					(tipState = TIP_HEATING);
#endif
			break;

		case BUTTON_OK_LONG:
			static uint32_t oldTempTargetDegC;

			oldTempTargetDegC = currentTempTargetDegC;	//保存当前温度
			if(!systemSettings.MenuKeepHeating)	//若进入菜单后停止加热
			{
				if(systemSettings.CalibrationEnable)
				currentTempTargetDegC = solveCubicEquations(0);
				else
					currentTempTargetDegC = 0;
			}

			enterSettingsMenu(); // enter the settings menu

			currentTempTargetDegC = oldTempTargetDegC;	//还原当前温度
			u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);
			u8g2.setFontRefHeightText();
			buttonLockout = true;
			break;
		case BUTTON_BOTH_LONG:
			heaterThermalRunaway = false;
		default:
			break;
		}




		//获取温度
		DegCTip = TipThermoModel::getTipInC();
		static uint16_t DegCTipAfterfliter = DegCTip;
		//模板参数如何传指针？？？？
		/**
		 * 模板参数:
		 * 在定义一个模板的时候需要在定义上面写上一行诸如template<typename T>这样的代码，
		 * 那么这行中定义的这个T就是一个模板参数。对于一个模板而言，无论是函数模板还是类模板，
		 * 都需要对其指定模板参数，可以有多个，但至少要有一个
		 *
		 * 每一个模板参数在正式使用的时候都必须是被指定的，虽然指定的方法可以不是显式指定，
		 * 但必须可以推导出来，换句话说编译器必须要能知道每个参数是什么才行
		 */
		if(systemSettings.kalmanQEnable)
			DegCTipAfterfliter = kalmanFilter(&KFP_Temp, (float) DegCTip);
		else
			DegCTipAfterfliter = DegCTip;
/*
		uint32_t previousState = xTaskGetTickCount();
		static uint32_t previousStateChange = xTaskGetTickCount();
		if ((previousState - previousStateChange) > (1000 / systemSettings.homeTipInCFPS)) {	///	这个500决定向父级菜单递归的阻塞感
			previousStateChange = previousState;
			//DegCTipAfterfliter = filter.average();
		}
*/

		static const uint8_t FONT16_XOFFSET = 20;
		tipDetected();

		if (!shouldBeSleeping(true)) {
			tipState = TIP_HEATING;
			if (systemSettings.CalibrationEnable)
				currentTempTargetDegC = solveCubicEquations(
										systemSettings.SolderingTemp);	//经校准还原设定温度
			else
				currentTempTargetDegC = systemSettings.SolderingTemp;	//还原设定温度
			//usb_printf("currentTempTargetDegC = %d\r\n", currentTempTargetDegC);
		}
		if ((tipState == TIP_HEATING) && systemSettings.SleepMode
				&& shouldBeSleeping()) {
			tipState = TIP_SLEEPING;
			if (systemSettings.CalibrationEnable) {
				currentTempTargetDegC = solveCubicEquations(
						min(systemSettings.SleepTemp,	// 把较低温度赋值给当前摄氏度
								systemSettings.SolderingTemp));
			} else {
				currentTempTargetDegC = min(systemSettings.SleepTemp,// 把较低温度赋值给当前摄氏度
						systemSettings.SolderingTemp);
			}
		}

		if ((tipState == TIP_SLEEPING) && shouldBeSleeping()
				&& shouldShutdown()) {
			tipState = TIP_SHUT_DOWN;
			if (systemSettings.CalibrationEnable) {
				currentTempTargetDegC = solveCubicEquations(0);
			} else {
				currentTempTargetDegC = 0;
			}
		}

#if 0	//单按ok休眠，，，
		if(tipState == TIP_HEATING && buttons ==  BUTTON_OK_SHORT)
		{
			lastButtonTime = 0;
			tipState = TIP_SLEEPING;
		}
		else
		{
			if(buttons ==  BUTTON_OK_SHORT)
			{
				lastButtonTime = xTaskGetTickCount() - getSleepTimeout();
				tipState = TIP_HEATING;
			}

		}
#endif

		//taskENTER_CRITICAL();
		u8g2.clearBuffer();

		if ((tipState != TIP_OPEN_CIRCUIT) && (tipState != TIP_HEATING)) {				//非heating模式显示两行16pixel字体

//					u8g2.setDrawColor(1);
//				    u8g2.drawBox(0, 0, 128, 32);
//					u8g2.setDrawColor(0);

			u8g2.setFont(u8g2_font_profont22_mr);	//12pixel 字间距
			u8g2.setFontRefHeightExtendedText();
			char buffer[6] = { 0 };

			uint16_t degCTip = (uint16_t)DegCTip;
			if(systemSettings.CalibrationEnable)	//已校准并使能校准数据
				degCTip = calibrationHorner(degCTip);

			sprintf(buffer, "%3d", degCTip);


			u8g2.drawStr(FONT16_XOFFSET + 12, -3, buffer);
			u8g2.drawXBM(FONT16_XOFFSET + 12 + 36, -1, 8, 16, symbolCelsius8x16);		//绘制℃符号

			if (tipState == TIP_SLEEPING) {
				u8g2.drawStr(FONT16_XOFFSET + 12, 15, "SLEEP");
				u8g2.drawXBM(FONT16_XOFFSET + 12 + 36 + 12, 0, 12, 16, symbolZZZ12x16);
#if 1
				// Draw symbolZZZ12x16 的遮挡矩形，呈现动画效果
				// Then draw over it, the bottom 5 pixels always stay. 8 pixels above that are
				// the levels masks the symbol nicely
				static uint8_t cnt = 0;
				uint32_t previousZzzState = xTaskGetTickCount();
				static uint32_t previousZzztateChange = xTaskGetTickCount();
				if ((previousZzzState - previousZzztateChange) > 500) {
					++cnt;
					cnt = cnt % 4;		//3 ranges
					previousZzztateChange = previousZzzState;
				}
				// Then we want to draw down (16-(5+state)
				u8g2.setDrawColor(0);
				u8g2.drawBox(FONT16_XOFFSET + 12 + 36 + 12, 0, 12,
						 (3 - cnt) * 5);	//若drawBox的高度<0，则会导致后续绘制的显示错位+乱码
				u8g2.setDrawColor(1);
				//usb_printf("cnt = %d\r\n" , cnt);
#endif
			}
			else	//tipState = TIP_SHUT_DOWN;
			{
				u8g2.setDrawColor(0);
				static uint8_t indexFanFrame = 0;
				u8g2.drawXBM(FONT16_XOFFSET + 12 + 36 + 12, -1, 16, 16,
						symbolFan16x16[indexFanFrame]);
				indexFanFrame = (indexFanFrame + 1) % 5;	//0~4
				u8g2.setDrawColor(1);
				u8g2.drawStr(FONT16_XOFFSET, 15, "COOLING");
			}
		}
#if 0
#endif

		//超时熄屏，动作唤醒（可接受按钮、运动、拔出烙铁三种动作）
		if (systemSettings.ShutDownMode
				&& (DegCTip < systemSettings.ShutDownTemp) 	//温度<100
				&& ((systemSettings.Sensitivity
						&& (((xTaskGetTickCount() - lastMovementTime)
								> MOVEMENT_INACTIVITY_TIME)
								&& ((xTaskGetTickCount() - lastButtonTime)
										> BUTTON_INACTIVITY_TIME)))
						|| tipState == TIP_SHUT_DOWN)) {

			u8g2.setFont(u8g2_font_profont22_mr);	//12pixel 字间距
			u8g2.setFontRefHeightExtendedText();
			u8g2.drawStr(FONT16_XOFFSET, 15, "PWR OFF");
			screenBrightness--;

			setContrast(*screenBrightness.val);
			if (*screenBrightness.val == 0) {
				u8g2.setPowerSave(1);
				firstScreenBright = true;
			} else
				u8g2.setPowerSave(0);
		} else {
			screenBrightness++;
			setContrast(*screenBrightness.val);
			u8g2.setPowerSave(0);
		}

		tipDetected();	//开路检查
		if (tipState == TIP_HEATING) {
			uint16_t degCTip = (uint16_t)DegCTipAfterfliter;
			if(systemSettings.CalibrationEnable)	//已校准并使能校准数据
				degCTip = calibrationHorner(DegCTipAfterfliter);
			printTipInCNumber(degCTip, 14);	//打样32pixel温度
		}
		//绘制两侧信息栏
		drawLeftParameters();
		drawRightParameters();

		//taskEXIT_CRITICAL();
		u8g2.sendBuffer();
		GUIDelay();

#if 0
			//usb打印按钮状态
			if (buttons)
				usb_printf("button state = %d\r\n", buttons);

			usb_printf("Tip DegC = %d  Handle DegC = %d\r\n",
					DegCTip,
					DegCHandle);

			usb_printf("Ij: %d, %d, %d, %d, %d, %d, %d, %d\r\n", count,
					hadc1.Instance->JDR1, hadc1.Instance->JDR2,
					hadc1.Instance->JDR3, hadc1.Instance->JDR4,
					hadc2.Instance->JDR1, hadc2.Instance->JDR2,
					hadc2.Instance->JDR3, hadc2.Instance->JDR4);
#endif
	}
}
void drawLeftParameters(int8_t Xcol)
{
	drawHeatSymbol(X10WattsToPWM(x10WattHistory.average()), Xcol);
	//usb_printf("PWM = %d\r\n", gui_pwm);
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//8pixel字体
	u8g2.setFontRefHeightText();
	u8g2.drawStr(Xcol, 16, "PW");
	uint16_t X10WattLimits = getX10WattageLimits();
	uint16_t pwDisp = x10WattHistory.average();
	if(systemSettings.PwDispMode == 0) //单位为瓦特
		pwDisp /= 10;
	else //单位为百分比
		pwDisp = map(pwDisp, 0, X10WattLimits, 0, 1000)/10;
	drawNumber((pwDisp > 9)?Xcol:(Xcol+1), 24, pwDisp, 2);
}
