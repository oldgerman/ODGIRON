/*
 * Page.cpp
 *
 *  Created on: 2021年4月20日
 *      Author: OldGerman
 */

/**
 * C++对象数组构造函数
 * 要使用需要多个参数的构造函数，则初始化项必须釆用函数调用的形式。
 * 例如，来看下面的定义语句，它为 3 个 Circle 对象的每一个调用 3 个参数的构造函数：
 * Circle circle[3] = {Circle(4.0, 2, 1),Circle(2.0, 1, 3),Circle (2.5, 5, -1) };
 * 没有必要为数组中的每个对象调用相同的构造函数。例如，以下语句也是合法的：
 * Circle circle [3] = { 4.0,Circle (2.0, 1, 3),2.5 };
 */
/**
 * array对象和数组存储在相同的内存区域（栈）中，vector对象存储在自由存储区（堆）
 */


#include <Page.hpp>
#include <string.h>
#include "Translation.h"
#include "stdio.h"
#include "configuration.h"
#include "dtostrf.h"

bool waitingToChooseOneFromTwo();
void colum_FeaturesUnrealized();
void colum_FuncNull();
void shutScreen();
uint16_t valColumTest = 200;
uint16_t valColumBool = 0;
const char *strChooseOneFromTwo = "OK       Cancel";
const char *strfeaturesNotRealized = "功能未实现！";
/**
 * 用于某些colums二选一
 * 默认打印当前选中的colum标题字符串，也可指定字符串
 */
bool colums_StrChooseOneFromTwo(bool featuresRealized = false, const char *str = nullptr) {
	u8g2.setDrawColor(1);
	u8g2.setFont(u8g2_simsun_9_fntodgironchinese); //12x12 pixels
	u8g2.clearBuffer();
	u8g2.sendBuffer();	//立即发送空buffer,消除残影和乱码
	uint8_t y = 0;
	if (featuresRealized)	//若功能实现
	{
		if(!str)
		{
			Colum *ptrColum = Page::ptrPage->getColumsSelected();
			//if(ptrColum->str != nullptr)
			u8g2.drawUTF8(1, y, ptrColum->str);	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
		}
		else {
			u8g2.drawUTF8(1, y, str);	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
		}
	} else {					//若未实现
		u8g2.drawUTF8(1, y, strfeaturesNotRealized);
	}
	u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
	y += 16;
	u8g2.drawUTF8(1, y, strChooseOneFromTwo);
	u8g2.sendBuffer();

	return waitingToChooseOneFromTwo();
}

/*打印未实现函数指针功能colums*/
void colum_FeaturesUnrealized() {
	colums_StrChooseOneFromTwo(false);
}
//不做任何动作
void colum_FuncNull() {
	;
}

/*
 * 更新温度可调上限
 * 影响关系：“->”"<-"表示单向影响 "<->"表示双向影响：
 * 	systemSettings.TempOffsetRef->tipDisconnectedThres
 * 	systemSettings.Calibration->tipDisconnectedThres
 * 	tipDisconnectedThres->tipTempMaxAdjust
 * 	tipTempMaxAdjust->systemSettings.SolderingTemp的上限值
 * 	systemSettings.TempOffsetRef->systemSettings.SolderingTemp的上限值和当前值
 */
void columsSoldringSettings_UpdateTipTempMaxAdjust();
std::vector<Colum> columsSoldringSettings = {
		Colum("开机焊接", &systemSettings.AutoStartMode, 1, 1, 0, 1, 1),
		Colum("菜单页加热", &systemSettings.MenuKeepHeating, 1, 1, 0, 1, 1),	//烙铁为常温?(OK Cancel) 偏置电压校准(正在自动校准.../校准完成, 结果：)
		Colum("目标温度",  &solderingTemp, "C", columsSoldringSettings_UpdateTipTempMaxAdjust, LOC_CHANGE),
		//Colum("目标温度",  columsSoldringSettings_UpdateTipTempMaxAdjust, LOC_ENTER),

		//Colum("温度负偏移", &systemSettings.TempOffsetRef, 2, 50, 0, 1, 10, "C",columsSoldringSettings_UpdateTipTempMaxAdjust, LOC_CHANGE),
		Colum("恒温休眠温度", &systemSettings.SleepTemp, 3, 300, 0, 1, 10, "C"),
		Colum("熄屏关断温度", &systemSettings.ShutDownTemp, 3, 300, 0, 1, 10, "C")
		//Colum("手柄超温保护")
};

void columsSoldringSettings_UpdateTipTempMaxAdjust() {
	tipDisconnectedThres = TipThermoModel::getTipMaxInC() - 5 ;//- systemSettings.TempOffsetRef;
	tipTempMaxAdjust = tipDisconnectedThres - 10;		   //焊接模式可调温度上限
	Colum *ptrColum = Page::ptrPage->getColumsSelected();
	if (ptrColum == &columsSoldringSettings.at(2))	//目标温度colum发生了修改
		ptrColum->ptrAutoValue->upper = tipTempMaxAdjust;
	else {	//温度负偏移colum发生了修改
		ptrColum--;	//选择目标温度的值
		if (*ptrColum->ptrAutoValue->val > tipTempMaxAdjust)	//若值高于上限
			*ptrColum->ptrAutoValue->val = tipTempMaxAdjust;	//则降低值为上限
	}
	Page::ptrPage->drawColums();	//实时更新
}

//休眠设置
AutoValue autoValueSleepTime(&systemSettings.SleepTime, 3, 999, 5, 1, 10);
AutoValue autoValueShutDownTime(&systemSettings.ShutDownTime, 3, 999, 5, 1, 10);
void columsDormantSettings_TimeOutToSleep() {
	tipState = TIP_SLEEPING;	//重置位于doGUITask()中for(;;)块内的tipState状态
}
std::vector<Colum> columsDormantSettings = {
		Colum("超时休眠", &systemSettings.SleepMode, 1, 1, 0, 1, 1, nullptr,columsDormantSettings_TimeOutToSleep, LOC_EXTI),
		Colum("超时休眠时间", &autoValueSleepTime, "S"),
		Colum("超时冷却时间", &autoValueShutDownTime, "S"),//实际的停止加热时间要减去休眠超时时间
		Colum("冷却后熄屏", &systemSettings.ShutDownMode, 1, 1, 0, 1, 1)
};

/**
 * 屏幕设置
 * Note：
 *  亮度为1仍有亮度，为0则熄屏
 */
void columsScreenSettings_Brightness() {
	screenBrightness.upper = systemSettings.ScreenBrightness;
	*screenBrightness.val = screenBrightness.upper;
	setContrast(*screenBrightness.val);
}

void columsScreenSettings_Orientation() {
#if 1
	switch (systemSettings.OrientationMode) {
	case 0:
	case 1:
		u8g2.setDisplayRotation(
				(systemSettings.OrientationMode == ORIENTATION_LEFT_HAND) ?
						U8G2_R2 : U8G2_R0);
		break;
	case 2:
		// do nothing on auto
		break;
	default:
		break;
	}
	Page::ptrPage->drawColums();
#endif
}

//std::map<uint16_t, const char*> columsScreenSettings_Orientation_Map = {{ 0, "A亮A"}, { 1, "B度B"}, { 2, "C旋C"}};
std::map<uint16_t, const char*> columsScreenSettings_Orientation_Map = {{ 0, "右手"}, { 1, "左手"}, { 2, "自动"}};
std::vector<Colum> columsScreenSettings = {
		Colum("亮度",		&systemSettings.ScreenBrightness, 3, 100, 1, 1, 10, "%", columsScreenSettings_Brightness, LOC_CHANGE),
		Colum("旋转模式",	&systemSettings.OrientationMode, 1, 2, 0, 1, 1, nullptr, columsScreenSettings_Orientation, LOC_EXTI,
				&columsScreenSettings_Orientation_Map),//0: Right 1:Left 2:Automatic
		Colum("开机显示图标", &systemSettings.PowerOnShowLogo, 1, 1, 0, 1, 1)
};

Page pageScreenSettings(&columsScreenSettings);	//因混乱跳转测试提前声明

//PID设置
uint16_t columsABCVal_D = 0;
uint16_t columsABCVal_E = 0;
uint16_t columsABCVal_F = 0;

std::vector<Colum> columsDEF = {
		Colum("D", &columsABCVal_D, 3, 100, 1, 1, 10, "W"),
		Colum("E", &columsABCVal_E, 3, 100, 1, 1, 10, "W"),
		Colum("F", &columsABCVal_F, 3, 100, 1, 1, 10, "W"),
		Colum("屏幕设置(test Mixed)", &pageScreenSettings) //混乱跳转测试

};

Page pageDEF(&columsDEF);

uint16_t columsABCVal_A = 0;
uint16_t columsABCVal_B = 0;
uint16_t columsABCVal_C = 0;
std::vector<Colum> columsABC = {
		Colum("A", &columsABCVal_A, 3, 100, 1, 1, 10, "W"),
		Colum("B", &columsABCVal_B, 3, 100, 1, 1, 10, "W"),
		Colum("C", &columsABCVal_C, 3, 100, 1, 1, 10, "W"),
		Colum("DEF(test)", &pageDEF)	//四级菜单测试
};

Page pageABC(&columsABC);



/**
 * Q和R分别代表对预测值和测量值的置信度（反比），通过影响卡尔曼增益K的值，影响预测值和测量值的权重。越大的R代表越不相信测量值。
 *
 */
std::vector<Colum> columsKFP = {
		Colum("启用滤波器",  &systemSettings.kalmanQEnable, 1, 1, 0, 1, 1),
		Colum("协方差矩阵Q", &systemSettings.kalmanQ, 3, 999, 1, 1, 10, "%"),
		Colum("协方差矩阵P", &systemSettings.kalmanP, 3, 999, 1, 1, 10, "%")
};

Page pageKFP(&columsKFP);	//因混乱跳转测试提前声明

std::vector<Colum> columsPID = {
		Colum("比例系数", &systemSettings.pidKp, 3, 100, 1, 1, 10, "%"),
		Colum("卡尔曼滤波器" , &pageKFP),
		Colum("目标温度正偏", &systemSettings.balanceTempOffsetPositive, 2, 99, 0, 1, 10, "C"),
		Colum("目标温度负偏", &systemSettings.balanceTempOffsetnegative, 2, 99, 0, 1, 10, "C"),
		Colum("输出功率限制", &systemSettings.powerLimit, 2, MAX_POWER_LIMIT, 1, 1, 10, "W"),
		Colum("维持功率X10", &systemSettings.KeepAwakePulse, 3, 100, 1, 1, 10, "W")/*,
		Colum("ABC(test)", &pageABC)	//三级菜单测试2022*/
};

//校准页面的
void columsCalibration_Calibrate()
{
//这个页面得加热啊，需要修改并在退出时恢复菜单页加热这个flash设置值
	//if(colums_StrChooseOneFromTwo(true))

	{
		const uint8_t xOffset = 74;
		static uint32_t oldTempTargetDegC;
		oldTempTargetDegC = currentTempTargetDegC;	//保存进入校准程序之前的PID目标温度

		//测试: 返回当前colum的是本页面的第几个，返回值例如0，1，2，3，4
		const int num_of_itrColum_offset = 2;//跳过前两个Colums
		int num_of_itrColum = Page::ptrPage->num_of_itrColum() - num_of_itrColum_offset;
		uint16_t setTempTargetDegCVal = 360;
		AutoValue setTempTargetDegC(&setTempTargetDegCVal, 3, tipTempMaxAdjust, 0, 1, 10, true);
		int ok = 0;

		switch (num_of_itrColum) {
		case 0:
			// 最低温度不适用
			*setTempTargetDegC.val = 0;
			setTempTargetDegC.lower = 0;
			setTempTargetDegC.upper = 0;
		case 1:
		case 2:
		case 3:
			*setTempTargetDegC.val = num_of_itrColum * 100;
			setTempTargetDegC.lower = (num_of_itrColum - 1) * 100;
			setTempTargetDegC.upper = (num_of_itrColum + 1) * 100;
			//校准每100度步幅度温度
			break;
		case 4: 			//校准最高温度
			*setTempTargetDegC.val = 360;
			setTempTargetDegC.lower = 300;
			setTempTargetDegC.upper = tipTempMaxAdjust;
			break;
		default:
			break;
		}

		if(systemSettings.CalibrationSetTempEnable && (num_of_itrColum != 0)) { // 最低温度不适用
			u8g2.clearBuffer();
			u8g2.setDrawColor(1);//黑底白字
			u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels中文字体
			u8g2.drawUTF8(1, 0, "设置校准的目标温度");	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
			u8g2.drawUTF8(1, 19, "目标温度");	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
			u8g2.drawXBM(xOffset + 36, 17, 8, 16, symbolCelsius8x16);		//绘制℃符号
			//绘制+-号
			u8g2.drawBox(121, 24, 7, 1);
			u8g2.drawBox(121+3, 24-3, 1, 7);
			u8g2.drawBox(121-57, 24, 7, 1);
			u8g2.setFont(u8g2_font_profont22_mr);	//12pixel 字间距英文数字字体
			for (;;) {
				buttons = getButtonState();
				if (buttons)
				AutoValue::buttonState = buttons;
				switch (buttons) {
				case BUTTON_B_LONG:
				case BUTTON_B_SHORT:
					setTempTargetDegC--;//AutoValue::operator--()在内部判断buttons长按短按
					break;
				case BUTTON_F_LONG:
				case BUTTON_F_SHORT:
					setTempTargetDegC++;
					break;
				case BUTTON_OK_SHORT:

					ok = -1;
					break;
				default:
					break;
				}

				char buffer2[6] = { 0 };
				sprintf(buffer2, "%3d", (uint16_t) *setTempTargetDegC.val);
				u8g2.drawStr(xOffset, 17, buffer2);
				u8g2.sendBuffer();
				GUIDelay();
				if(ok == -1)
					break;
			}
		}

		currentTempTargetDegC = *setTempTargetDegC.val;

		u8g2.clearBuffer();
		u8g2.setDrawColor(1);//黑底白字
		u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels中文字体
		u8g2.drawUTF8(1, 1, "当前温度");	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
		u8g2.drawUTF8(1, 19, "校准温度");	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码

		u8g2.drawXBM(xOffset + 36, -1, 8, 16, symbolCelsius8x16);		//绘制℃符号
		u8g2.drawXBM(xOffset + 36, 17, 8, 16, symbolCelsius8x16);		//绘制℃符号
		//绘制+-号
		u8g2.drawBox(121, 24, 7, 1);
		u8g2.drawBox(121+3, 24-3, 1, 7);
		u8g2.drawBox(121-57, 24, 7, 1);
		u8g2.setFont(u8g2_font_profont22_mr);	//12pixel 字间距英文数字字体


		Colum *ptrColum = Page::ptrPage->getColumsSelected();
		AutoValue *ptrAutoValue = ptrColum->ptrAutoValue;
		*(ptrColum->ptrAutoValue)->val = *setTempTargetDegC.val; //同步默认校准的温度位设置的目标温度
		for (;;) {

			uint16_t DegCTip = TipThermoModel::getTipInC();
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


			char buffer[6] = { 0 };
			sprintf(buffer, "%3d", (uint16_t) DegCTipAfterfliter);

			buttons = getButtonState();
			if (buttons)
			AutoValue::buttonState = buttons;
			switch (buttons) {
			case BUTTON_B_LONG:
			case BUTTON_B_SHORT:
				(*ptrAutoValue)--;//AutoValue::operator--()在内部判断buttons长按短按
				break;
			case BUTTON_F_LONG:
			case BUTTON_F_SHORT:
				(*ptrAutoValue)++;
				break;
			case BUTTON_OK_SHORT:
				systemSettings.calx[num_of_itrColum] = DegCTipAfterfliter;	//存储x数据点：当前温度
				currentTempTargetDegC = oldTempTargetDegC;	//离开时还原到进入校准程序之前的PID目标温度
				return;
			default:
				break;
			}

			char buffer2[6] = { 0 };
			sprintf(buffer2, "%3d", (uint16_t) *ptrAutoValue->val);

			//taskENTER_CRITICAL();	//不要临界段，会导致PID超调
			u8g2.drawStr(xOffset, -1, buffer);
			u8g2.drawStr(xOffset, 17, buffer2);
			//taskEXIT_CRITICAL();
			u8g2.sendBuffer();
			GUIDelay();
		}
  }
}

//秦九韶算法
double calibrationHorner(uint16_t x_cal/*键入计算变量值*/, double (&a)[CAL_M]/*多项式系数*/,
		double X/*x数组元素平均值*/, int nc /*多项式的次数*/) {
    double *a_in = new double[nc + 1];  //n次多项式申请n+1大小的数组
    //多项式的系数（最高次项开始）
    for (int i = nc; i >= 0; i--)
      a_in[i] = a[i];  //读入各项系数

    double ans = a_in[nc];

    for (int i = nc - 1; i >= 0; i--)
      ans = ans * (x_cal - X) + a[i];  //最高次项开始，往外展开

    delete[] a_in;  //释放动态内存

    return ans;
}

//使能校准数据
void columsCalibration_Enable() {
	bool ok = colums_StrChooseOneFromTwo(true);
	if (ok) {
		systemSettings.CalibrationEnable = 1;
	}
}

//查看校准数据
void columsCalibration_Show() {
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_IPAandRUSLCD_tf);	//8pixel字体
	char buffer[6] = { 0 };
	const uint8_t num_buffer2 = 18; //15位+小数点+正负号+结束空字符
	char buffer2[num_buffer2] ={0};
	//char buffer2[num_buffer2] = "187.124567890123";

	int8_t ok = -1;

	for (;;) {
		//taskENTER_CRITICAL();//临界段代码，防止绘图时被打乱出现错位
		u8g2.setDrawColor(1);//黑底白字
		for (int i = 0; i < 4; i++) {
			sprintf(buffer, "a%1d", i);
			u8g2.drawStr(2, i * 8, buffer);
			memset(buffer2, 0, strlen(buffer2));
			dtostrf(systemSettings.cala[i], 15, 10, buffer2, num_buffer2); // Convert floating point to char
			//dtostrf((double)-187.1245, 10, 5, buffer2, num_buffer2); // Convert floating point to char
			u8g2.drawStr(41, i * 8, buffer2);

		}
		//taskEXIT_CRITICAL();
		u8g2.sendBuffer();

		buttons = getButtonState();
		switch (buttons) {
		case BUTTON_B_SHORT:
		case BUTTON_F_SHORT:
			ok = 0;
			break;
		default:
			break;
		}
		//每1s 扫描/清除 切换一次
		if (ok != -1)
			break;
		GUIDelay();
	}
	ok = -1;
	u8g2.clearBuffer();
	for (;;) {
		//taskENTER_CRITICAL();//临界段代码，防止绘图时被打乱出现错位
		const char *str[] = {"X","x","y","c"};
		for (int i = 0; i < 4; i++) {
		    memset(buffer, 0, strlen(buffer));
			sprintf(buffer,str[i]);
			u8g2.drawStr(2, i * 8, buffer);
		}
		//calX:
		memset(buffer2, 0, strlen(buffer2));
		dtostrf(systemSettings.calX, 15, 10, buffer2, num_buffer2);
		u8g2.drawStr(15, 0, buffer2);
		//calx y c:
		for(int cnt = 0; cnt < 3; cnt++){
			for (int i = 0; i < CAL_N; i++) {
				memset(buffer, 0, strlen(buffer));
				switch (cnt) {
				case 0:
					sprintf(buffer, "%3d", systemSettings.calx[i]);
					break;
				case 1:
					sprintf(buffer, "%3d", systemSettings.caly[i]);
					break;
				case 2:
					static uint16_t calc;
					calc = calibrationHorner(systemSettings.calx[i], systemSettings.cala, systemSettings.calX, CAL_N-1);
					sprintf(buffer, "%3d", calc);
					break;
				default:
					break;
				}
				u8g2.drawStr(15 + i * 24, (cnt + 1) * 8, buffer);
			}
		}
		//taskEXIT_CRITICAL();
		u8g2.sendBuffer();

		buttons = getButtonState();
		switch (buttons) {
		case BUTTON_B_SHORT:
		case BUTTON_F_SHORT:
			ok = 0;
			break;
		default:
			break;
		}
		//每1s 扫描/清除 切换一次
		if (ok != -1)
			break;
		GUIDelay();
	}

}

/******************************************
 //参考 《常用算法程序集 （C语言描述 第三版)》

 //最小二乘法
x[n] y[n] : 数据点数据
n: 输入点个数
a[m]: 返回m-1次拟合多项式的m个系数
m: 拟合多项式的项数,即拟合多项式的最高次为m-1：没有问题

//拟合多项式的输出：其中X为已知点x的平均值
Y(x) = a0 + a1(x-X) + a2(x-X)^2 + …… am(x-X)^m
 ******************************************/
void calibrationCalculate(
		uint16_t (&x)[CAL_N], uint16_t (&y)[CAL_N],	double (&a)[CAL_M], //引用传递数组需要将&array括起来，然后加上数组元素个数
		double &X, int n = CAL_N, int m = CAL_M) {
	   X = 0.0;	//必须要归零一次，不然会继承上一次的数据
	  for (int i = 0; i < n; i++) {
	    X += x[i];
	  }
	  X /= n;

	  int i, j, k;
	  double z, p, c, g, q, d1, d2, s[20], t[20], b[20];

	  for (i = 0; i <= m - 1; i++)
	    a[i] = 0.0;

	  if (m > n) m = n;
	  if (m > 20) m = 20;

	  z = 0.0;

	  for (i = 0; i <= n - 1; i++)
	    z = z + x[i] / (1.0 * n);

	  b[0] = 1.0;
	  d1 = 1.0 * n;
	  p = 0.0;
	  c = 0.0;
	  for (i = 0; i <= n - 1; i++) {
	    p = p + (x[i] - z);
	    c = c + y[i];
	  }
	  c = c / d1;
	  p = p / d1;
	  a[0] = c * b[0];

	  //if (m > 1) {
	    t[1] = 1.0;
	    t[0] = -p;
	    d2 = 0.0;
	    c = 0.0;
	    g = 0.0;
	    for (i = 0; i <= n - 1; i++) {
	      q = x[i] - z - p;
	      d2 = d2 + q * q;
	      c = c + y[i] * q;
	      g = g + (x[i] - z) * q * q;
	    }
	    c = c / d2;
	    p = g / d2;
	    q = d2 / d1;
	    d1 = d2;
	    a[1] = c * t[1];
	    a[0] = c * t[0] + a[0];
	  //}

	  for (j = 2; j <= m - 1; j++) {
	    s[j] = t[j - 1];
	    s[j - 1] = -p * t[j - 1] + t[j - 2];
	    if (j >= 3)
	      for (k = j - 2; k >= 1; k--)
	        s[k] = -p * t[k] + t[k - 1] - q * b[k];
	    s[0] = -p * t[0] - q * b[0];
	    d2 = 0.0;
	    c = 0.0;
	    g = 0.0;
	    for (i = 0; i <= n - 1; i++) {
	      q = s[j];
	      for (k = j - 1; k >= 0; k--)
	        q = q * (x[i] - z) + s[k];
	      d2 = d2 + q * q;
	      c = c + y[i] * q;
	      g = g + (x[i] - z) * q * q;
	    }
	    c = c / d2;
	    p = g / d2;
	    q = d2 / d1;
	    d1 = d2;
	    a[j] = c * s[j];
	    t[j] = s[j];
	    for (k = j - 1; k >= 0; k--) {
	      a[k] = c * s[k] + a[k];
	      b[k] = t[k];
	      t[k] = s[k];
	    }
	  }
}


//保存校准数据
void columsCalibration_Save() {
	bool ok = colums_StrChooseOneFromTwo(true);
	if (ok) {
		  calibrationCalculate(systemSettings.calx, systemSettings.caly, systemSettings.cala, systemSettings.calX);
		  systemSettings.Calibrated = true;
		  saveSettings();
		  restoreSettings();
	}
}


//清除校准数据
void columsCalibration_Reset() {
	bool ok = colums_StrChooseOneFromTwo(true);
	if (ok) {
		calibrationReset();
	}
}

//uint16_t numCalibTest = 100;	//临时测试值
//负数符号显示？？？算了不显示，只显示校准参考的温度
std::vector<Colum> columsCalibration = {
		Colum("启用校准数据", &systemSettings.CalibrationEnable, 1, 1, 0, 1, 1),
		Colum("设置目标温度", &systemSettings.CalibrationSetTempEnable, 1, 1, 0, 1, 1),
		Colum("校准最低温度", &systemSettings.caly[0],  3, 100, 0, 1, 10, "C", columsCalibration_Calibrate, LOC_ENTER),
		Colum("校准第二档位",   &systemSettings.caly[1], 3, 200, 0, 1, 10, "C", columsCalibration_Calibrate, LOC_ENTER),
		Colum("校准第三档位",   &systemSettings.caly[2], 3, 300, 100, 1, 10, "C", columsCalibration_Calibrate, LOC_ENTER),
		Colum("校准第四档位",   &systemSettings.caly[3], 3, 400, 200, 1, 10, "C", columsCalibration_Calibrate, LOC_ENTER),
		Colum("校准最高温度", &systemSettings.caly[4],  3, 500, 300, 1, 10, "C", columsCalibration_Calibrate, LOC_ENTER),
		Colum("保存校准值", columsCalibration_Save, LOC_ENTER),
		Colum("查看校准数据", columsCalibration_Show, LOC_ENTER),
		Colum("复位校准值", columsCalibration_Reset, LOC_ENTER)
};


//语言
/**
 * 语言互斥值更新
 * Uinicode字符：
 * $221a, $2713,对应：✓ ，$2713的✓宋体无法生成，$221a的√可以
 * $0020 对应半角空格
 */
std::map<uint16_t, const char*> columsLanguage_Map = {{ 0, " "}, { 1, "√"}};
void columsLanguage_MutexUpdate() {
	//得到行列打勾...
	//uint8_t cntSelected = *Page::indexColums.val / Page::indexColums.shortSteps;
	Colum *ptrColum;
	Colum *const ptrColumSelected = Page::ptrPage->getColumsSelected();
	*ptrColumSelected->ptrAutoValue->val = 1;
	ptrColum = Page::ptrPage->_listColums.front();
	//list::size(是C++ STL中的內置函數，用於查找列表容器中存在的元素數)
	uint8_t listSize = Page::ptrPage->_listColums.size();
	for (int i = 0; i < listSize; i++) {
		if (ptrColum == ptrColumSelected)
			systemSettings.Language = i;	//修改语言编号
		else
			*ptrColum->ptrAutoValue->val = 0;
		ptrColum++;
	}
	Page::ptrPage->drawColums();	//实时更新
}

//生成了一个具有LANGUAGES_NUM个uint16_t型值的列表，并且每一个值都等于0
std::vector<uint16_t> autoValueLanguageVals(LANGUAGES_NUM, 0);
std::vector<uint16_t> *ptrAutoValueLanguageVals = &autoValueLanguageVals;
//at()访问vector容器中单个元素
AutoValue autoValueLanguageZh(&autoValueLanguageVals.at(0), 1, 1, 1, 1, 1);	//注意最小最大值都为1，仅允许用户从0修改为1，从1修改为0由columsLanguage_MutexUpdate()执行
AutoValue autoValueLanguageEn(&autoValueLanguageVals.at(1), 1, 1, 1, 1, 1);
AutoValue autoValueLanguageJp(&autoValueLanguageVals.at(2), 1, 1, 1, 1, 1);
std::vector<Colum> columsLanguage = {
		Colum("中文", &autoValueLanguageZh, nullptr, columsLanguage_MutexUpdate, LOC_ENTER, &columsLanguage_Map),
		Colum("English", &autoValueLanguageEn, nullptr, columsLanguage_MutexUpdate, LOC_ENTER, &columsLanguage_Map),
		Colum("日本語", &autoValueLanguageJp, nullptr, columsLanguage_MutexUpdate, LOC_ENTER, &columsLanguage_Map)
};
Page pageLanguage(&columsLanguage);

//菜单交互设置
std::vector<Colum> columsMenuMotionControl = {
		Colum("运动检测", &colum_FeaturesUnrealized),	//开/关
		Colum("垂直基准夹角", &colum_FeaturesUnrealized),
		Colum("触发角度阈值", &colum_FeaturesUnrealized),
};
Page pageMenuMotionControl(&columsMenuMotionControl);

std::map<uint16_t, const char*> columsMenuInteraction_Iteration_Map = {{ 0, "单向"}, { 1, "循环"}};
std::map<uint16_t, const char*> columsMenuInteraction_SelectedDisplay_Map = {{ 0, "反显"}, { 1, "矩形"}};
std::vector<Colum> columsMenuInteraction = {
		Colum("滚动条", &colum_FeaturesUnrealized),
		Colum("迭代方向", &colum_FeaturesUnrealized),		//单向/循环
		Colum("跟随运动迭代", &pageMenuMotionControl),		//下级菜单
		Colum("选中显示方式", &colum_FeaturesUnrealized),		//反显/矩形
		Colum("保持最后选中", &colum_FeaturesUnrealized),
		Colum("反向递归延时", &colum_FeaturesUnrealized),
		Colum("编辑超时退出", &colum_FeaturesUnrealized),
		Colum("菜单超时退出", &colum_FeaturesUnrealized)
};
Page pageMenuInteraction(&columsMenuInteraction);

//其他参数设置
std::vector<Colum> columsOtherParameters = {
		Colum("加速度计阈值", &systemSettings.Sensitivity, 3, 100, 1, 1, 10, "%"),
		Colum("蜂鸣器音量", &systemSettings.BuzzerVolume, 3, 100, 0, 1, 10, "%")/*,
		Colum("菜单交互", &pageMenuInteraction),
		Colum("语言", &pageLanguage)//2022*/
};

#ifdef ResetToDFU
//辅助功能
void columsAccessibility_ResetToDFU() {
	//向FLashEEPROM写入APP跳转回DFU的标志位，该情况bootloader不打印logo，无需按键直接进入DFU模式
	/* .... */
	bool ok = colums_StrChooseOneFromTwo(true);
	if (ok) {
		systemSettings.ResetForceDFU = 1;	//更改标记，DFU程序检测此标记
		saveSettings();
		shutScreen();
		//osDelay(1000);	//必要的延时等USB设备复位
		NVIC_SystemReset();
	}
}
#endif

void columsAccessibility_ResetSettings() {
	//向FLashEEPROM写入APP跳转回DFU的标志位，该情况bootloader不打印logo，无需按键直接进入DFU模式
	/* .... */
	bool ok = colums_StrChooseOneFromTwo(true);
	if (ok) {
		//shutScreen();
		resetSettings();
		saveSettings();
	}
}

void columsAccessibility_I2CScaner() {
	u8g2.clearBuffer();
	u8g2.setDrawColor(1);
	u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels
	uint8_t y = 0;

	Colum *ptrColum = Page::ptrPage->getColumsSelected();
	u8g2.drawUTF8(1, y, ptrColum->str);	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码

#ifdef STM32F1
	I2C_HandleTypeDef* hi2cPtr = &hi2c2;
#elif defined(STM32F4)
	I2C_HandleTypeDef *hi2cPtr = &hi2c3;
#else
#endif
	uint8_t i = 0;
	HAL_StatusTypeDef status;
	int8_t ok = -1;
	char buffUSB[5] = { 0 };
	bool exchangeSate = false;	//标记: 扫描/清除
	bool firstIn = true; 		//标记: 第一次进入
	u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
	uint8_t i2cDevices = 0;

	for (;;) {
		buttons = getButtonState();
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
		//每1s 扫描/清除 切换一次

		uint32_t previousState = xTaskGetTickCount();
		static uint32_t previousStateChange = xTaskGetTickCount();
		if ((previousState - previousStateChange) > 1500 || firstIn) {
			firstIn = false;	//修改首次进入标记
			previousStateChange = previousState;
			exchangeSate = !exchangeSate;
			u8g2.setDrawColor(0);
			u8g2.drawBox(0, 15, 128, 16);
			u8g2.setDrawColor(1);
			if (exchangeSate)
				u8g2.drawStr(0, 15, "Scaning...");
			else {
				uint8_t xAddr = 0;
				for (i = 0; i < 127; i++) {
					status = HAL_I2C_Master_Transmit(hi2cPtr, i << 1, 0, 0,
							200);

					if (status == HAL_OK && i != 0) {
						++i2cDevices;	//设备计数+1
						sprintf(buffUSB, "0x%02X", i);
						u8g2.drawStr(xAddr, 15, buffUSB);
						xAddr += 40;	//最多三个地址
					}
					//osDelay(10);	//会造成 扫描/清除 延迟不对等
					//HAL_Delay(10);	//会死机
				}
				memset(buffUSB, 0, 5);
				if (!i2cDevices)	//未扫描到设备
					u8g2.drawStr(0, 15, "Nothing!");
			}
		}
		if (ok != -1)
			break;
		u8g2.sendBuffer();
		GUIDelay();
	}
}

// 偏置电压校准
void columsAccessibility_SetTipOffset() {
	if(colums_StrChooseOneFromTwo(true, "请将烙铁冷却到室温！"))
	{
		u8g2.clearBuffer();
		u8g2.setDrawColor(1);

		u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels
		uint8_t y = 0;
		Colum *ptrColum = Page::ptrPage->getColumsSelected();
		u8g2.drawUTF8(1, y, ptrColum->str);	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码

		int8_t ok_2 = -1;
		bool exchangeSate = false;	//标记: 扫描/清除
		bool firstIn = true; 		//标记: 第一次进入
		u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels

		// 注意这里置0，那么接下来的TipThermoModel::convertTipRawADCTouV()减去的校准偏差就是0
		systemSettings.CalibrationOffset = 0;
		uint16_t temporaryCalibrationOffset = 0;	//临时储存校准值
		uint8_t i = 0;	//采样次数
		uint32_t offset = 0;	//采样累计值
		bool enGetButtonState = false;	//校准状态禁用按键，显示结果状态可以使用按键退出
		for (;;) {
			if(enGetButtonState)
				buttons = getButtonState();
			switch (buttons) {
			case BUTTON_B_SHORT:
				ok_2 = 1;
				break;
			case BUTTON_F_SHORT:
				ok_2 = 0;
				break;
			default:
				break;
			}
			//每1s 扫描/清除 切换一次

			//校准累加,执行16次【8次过采样】并求和
			offset += getTipRawTemp(1);
			// cycle through the filter a fair bit to ensure we're stable.
			// 循环通过过滤器以确保我们稳定。
			i %= 16;	//i = 0~15, 16次迭代

			if(i == 0)
			//将8次过采样16次尖端温度的求和再求均值传入TipThermoModel::convertTipRawADCTouV()得到校准偏差为0时，
			//烙铁正负极由于上拉电阻产生的分压，这个值用作新的校准偏差值
			temporaryCalibrationOffset = TipThermoModel::convertTipRawADCTouV(
					offset / 16);


			uint32_t previousState = xTaskGetTickCount();
			static uint32_t previousStateChange = xTaskGetTickCount();
			if (((previousState - previousStateChange) > 1500 || firstIn) && i == 0) {
				firstIn = false;	//修改首次进入标记，首次进入立即打印“校准中，请稍等..."
				previousStateChange = previousState;
				exchangeSate = !exchangeSate;
				u8g2.setDrawColor(0);
				u8g2.drawBox(0, 15, 128, 16);	//黑色矩形清理第二行Colum
				u8g2.setDrawColor(1);
				// 如果热电偶位于尖端的末端，并且手柄位于是衡温，则输出应为零，因为没有温度差。
				// If the thermo-couple at the end of the tip, and the handle are at
				// equilibrium, then the output should be zero, as there is no temperature
				// differential.
				// systemSettings.CalibrationOffset = 0 说明校准完成
				if (exchangeSate || temporaryCalibrationOffset == 0)
				{
					u8g2.drawStr(0, 15, "Calibrating...");
					enGetButtonState = false;
				}
				else {
					offset = 0;
					systemSettings.CalibrationOffset = temporaryCalibrationOffset;
					temporaryCalibrationOffset = 0;
					// systemSettings.CalibrationOffset == 0
					u8g2.drawStr(0, 15, "Result:");
					Page::drawNumber(113 - 3* 6, 16, systemSettings.CalibrationOffset, 3);
					enGetButtonState = true;
				}
			}
			if (ok_2 != -1)
				break;
			u8g2.sendBuffer();
			osDelay(100);
			++i;
		}
	}
}

#include "policy_engine.h"
#include "BSP_PD.h"
void columsAccessibility_enumPDO()
{
	int8_t ok = colums_StrChooseOneFromTwo(true, "上电复位后才能枚举!");
	if(ok)
	{
		u8g2.clearBuffer();
		u8g2.sendBuffer();

		u8g2.setFont(u8g2_font_IPAandRUSLCD_tf); //迷你字体

		uint8_t indexColums = 0;
		uint8_t x = 0;
		int16_t voltage = 0;
		int16_t current  = 0;
		u8g2.drawStr(0, indexColums * 8, "Available V and I:");
		ok = -1;
		for(;;)
		{
			for(int i = 0; i < NUM_PDO_LEVELS / 2; i++)
			{
				buttons = getButtonState();
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

				char buf[23] = { 0 };
				voltage = USB_PD_PDO_Levels[(i * 2) + 0];
				current = USB_PD_PDO_Levels[(i * 2) + 1];
				if(voltage != 0)
				{
					sprintf(buf, "%2dV/%1d.%-1d%-1dA", voltage / 1000, current / 100, (current - 100 *(current / 100)) / 10, (current - 100 *(current / 100)) % 10);
					u8g2.drawStr(x, 8 + indexColums * 8, buf);
				}
				indexColums = (indexColums + 1) % 3;
				if(indexColums == 0 && x == 0)
					x = 64;
				else if(indexColums == 0 && x == 64)
					x = 0;
				else
					;
			}
			u8g2.sendBuffer();
			osDelay(30);

			if (ok != -1)
				break;
		}
	}
}

//重启
void columsHome_Reset()
{
	bool ok = colums_StrChooseOneFromTwo(true);
	if(ok)
	{
		shutScreen();

		//在这个时间内按下中键不放可以进入DFU
		uint16_t timeWaitingStartPage = HAL_GetTick();
		for (;;) {
			if (HAL_GetTick() - timeWaitingStartPage > 666)
				break;
			resetWatchdog();
			GUIDelay();
		}

		NVIC_SystemReset();
	}
}

std::vector<Colum> columsAccessibility = {
		Colum("偏置电压校准", 	columsAccessibility_SetTipOffset,LOC_ENTER),	//烙铁为常温?(OK Cancel) 偏置电压校准(正在自动校准.../校准完成, 结果：)
		Colum("扫描I2C设备", 	columsAccessibility_I2CScaner, LOC_ENTER),
		Colum("枚举PD挡位", 	columsAccessibility_enumPDO, LOC_ENTER),
		Colum("重启", columsHome_Reset),
		//Colum("重启进入DFU", 	columsAccessibility_ResetToDFU, LOC_ENTER),
		//Colum("USB虚拟串口",	&valColumBool, 1, 1, 0, 1, 1),
		Colum("恢复出厂设置",	columsAccessibility_ResetSettings, LOC_ENTER)
};





//Colum("旋转模式",	&systemSettings.OrientationMode, 1, 2, 0, 1, 1, nullptr, columsScreenSettings_Orientation, LOC_EXTI, &columsScreenSettings_Orientation_Map)

//版本信息
std::vector<Colum> columsVersionInformation = {
		Colum("ODGIRON-Firmware-v1.0", colum_FuncNull, LOC_ENTER),
		Colum("By: OldGerman", colum_FuncNull, LOC_ENTER)
};

Page pageSoldringSettings(&columsSoldringSettings);
Page pageDormantSettings(&columsDormantSettings);
//Page pageScreenSettings(&columsScreenSettings);
Page pageAccessibility(&columsAccessibility);
Page pagePID(&columsPID);
Page pageCalibration(&columsCalibration);
Page pageOtherParameters(&columsOtherParameters);
Page pageVersionInformation(&columsVersionInformation);



void columsHome_ShowVerInfo() {
	drawLogoAndVersion('A');
	u8g2.sendBuffer();

	bool getBack = false;
	bool state = waitingToChooseOneFromTwo();
	if (state == 0)
		getBack = true;
	if (!getBack) {
		//第二页
		u8g2.clearBuffer();
		for (int i = 0; i < 4; i++) {
			osDelay(10);
			u8g2.drawStr(0, i * 8, DebugMenu[i]);
		}
		u8g2.sendBuffer();
		waitingToChooseOneFromTwo();
	}
}


std::vector<Colum> columsHome = {
		Colum("焊接设置", &pageSoldringSettings),
		Colum("休眠设置", &pageDormantSettings),
		Colum("显示设置", &pageScreenSettings),
		Colum("温控设置", &pagePID),
		Colum("温度校准", &pageCalibration),
		Colum("其他参数设置", &pageOtherParameters),
		Colum("辅助功能", &pageAccessibility),
		Colum("版本信息", columsHome_ShowVerInfo)
};

Page pageHome(&columsHome);
Page *Page::ptrPage = &pageHome;	//初始化共用Page指针为起始页
Page *Page::ptrPagePrev;// = &pageHome;	//初始化上一次的Page指针为起始页
std::list<Page *> Page::ptrPageList;
Page *Page::homePage = &pageHome;
bool Page::timeOut = false;
uint16_t indexColumsValue = 0;
AutoValue Page::indexColums(&indexColumsValue, 2, 16, 0, 16, 0, false);
uint8_t Page::valIndex = 0;
//uint8_t Page::bbb = 0;			//按钮状态，debug
