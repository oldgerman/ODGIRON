/*
 * Page.hpp
 *
 *  Created on: 2021年4月20日
 *      Author: PSA
 */

#ifndef INC_PAGE_HPP_
#define INC_PAGE_HPP_
#include "Colum.hpp"
#include <list>
#include <vector>
#include "oled_init.h"
#include "font24x32numbers.h"
#include "Settings.h"
#include "Threads.hpp"
#include <Buttons.hpp>
#include "u8g2_simsun_9_fntodgironchinese.h"
#include "Arduino.h" //提供bit操作宏
#define mymax(a,b) ((a) > (b) ? (a) : (b))
#define mymin(a,b) ((a) < (b) ? (a) : (b))

#define NON_CATTOON 1
#ifdef __cplusplus
#include <algorithm>
#include <cstring>
void cartoonFreshColums(bool Dir, uint8_t Steps = 4);
//秦九韶算法
double calibrationHorner(uint16_t x_cal/*键入计算变量值*/, double (&a)[CAL_M] = systemSettings.cala/*多项式系数*/,
		double X = systemSettings.calX/*x数组元素平均值*/, int nc = CAL_M - 1 /*多项式的次数*/);

class Page {
public:
	Page() {
	}
	Page(std::vector<Colum> *columVec) //默认指向的上一个Page是自己
			{
		_numColums = columVec->size();				//获取colums个数
		std::vector<Colum>::iterator columIter;
		for (columIter = columVec->begin(); columIter != columVec->end();
				columIter++)
			_listColums.push_back(&(*columIter));	//std::list元素为Colum对象指针
		_itrColums = _listColums.begin(); //将选中的栏还原为第一个
	}
	virtual ~Page() {
	}
	static void flashPage() {
		bool firshInflashPage = 1;
		bool swapCartoonFreshColumsRec;
		//valIndex = *(indexColums.val);
		//swapCartoonFreshColumsRec =  valIndex ? 0 : 1 ;	//进入菜单检测标记，根据valIndex决定
		ptrPageList.push_back(ptrPage);					//进入菜单首先把honePage对象添加到链表末尾
		//体感控制：进入时采集x数据作为基准角度
		int16_t angleValVerticalRef = axAvg.avgx;
		uint16_t xErrorMap2Time = 200;
		for (;;) {
			valIndex = *(indexColums.val);
			swapCartoonFreshColumsRec =  valIndex ? 0 : 1 ;	//进入菜单检测标记，根据valIndex决定
			buttons = getButtonState();
			bool changeCartoonColums = false;
			switch (buttons) {
			case BUTTON_B_SHORT:
			case BUTTON_B_LONG:
#if NON_CATTOON
				if (*(ptrPage->_itrColums) == ptrPage->_listColums.back()) {
					ptrPage->_itrColums = ptrPage->_listColums.begin();
					indexColums--;
				}else{
					ptrPage->_itrColums++;
					indexColums++;
				}
				changeCartoonColums = true;
#endif
				break;
			case BUTTON_F_SHORT:
			case BUTTON_F_LONG:
#if NON_CATTOON
				if (ptrPage->_itrColums == ptrPage->_listColums.begin()) {
					ptrPage->_itrColums = ptrPage->_listColums.end();
					ptrPage->_itrColums--;
					indexColums++;
				}else{
					ptrPage->_itrColums--;
					indexColums--;
				}
				changeCartoonColums = true;
#endif
				break;
			case BUTTON_OK_LONG:
				break;
			case BUTTON_OK_SHORT:
#define DoubleMenu 0
#if DoubleMenu
				/*仅支持二级菜单*/
				if (ptrPage == homePage)	//仅对HomePage有效
				{
					ptrPage = (*ptrPage->_itrColums)->nextPage; //当前页面指针指向当前页面指针指向的Colum的下级菜单
					resetPageIndex(false);
				} else {
					//执行colums的改值函数
					columValAdjust(*ptrPage->_itrColums);
				}
#else
				/*支持三级及以上菜单*/
				if ((*ptrPage->_itrColums)->nextPage != nullptr)
				{
					if(firshInflashPage) {
						firshInflashPage = 0;
					}else{
						ptrPageList.push_back(ptrPage);
					}

					ptrPage = (*ptrPage->_itrColums)->nextPage; //当前页面指针指向当前页面指针指向的Colum的下级菜单
					resetPageIndex(false);
#if 0
						uint32_t _previousStateChange = xTaskGetTickCount();
						for(;; ){
							uint32_t _previousState = xTaskGetTickCount();
							if ((_previousState - _previousStateChange) > 800) {	///	这个500决定向父级菜单递归的阻塞感
								break;
							}
						}
#endif
				}
				else
				{
					Colum *ptrColum = ptrPage->getColumsSelected();
					if (ptrColum->funPtr != nullptr && ptrColum->ptrAutoValue == nullptr)
							ptrColum->funPtr();	//跳去执行函数指针的函数
					else
					//执行colums的改值函数
						columValAdjust(*ptrPage->_itrColums);
				}
#endif
				break;
			default:
				break;
			}

#if 0
			//体感控制
			/*
			 * 映射关系：xError小，则迭代时间短，反之亦然
			 */
			static const uint16_t angleValThreshold = 120;
			static const uint16_t angleDoubleScope = 1024 - angleValThreshold;	//阈值边界两侧可感角度最大范围绝对值
			//求补操作，即取反后加一
			if(axisData.x > angleValVerticalRef + angleValThreshold)
			{
				int16_t val = axisData.x - (angleValVerticalRef + angleValThreshold);
				val = mymin(val, angleDoubleScope);
				xErrorMap2Time = map(val , 0, angleDoubleScope, 300, 10);
				if (*(ptrPage->_itrColums) != ptrPage->_listColums.back()) {
					if(stateTimeOut2(xErrorMap2Time)) {
						ptrPage->_itrColums++;
						indexColums++;
						changeCartoonColums = true;
						buttons = BUTTON_B_SHORT;
					}
					//int16_t xError = axAvg.avgx - angleValVerticalRef;
				}
			}
			else if(axisData.x < angleValVerticalRef - angleValThreshold)
			{
				int16_t val = (angleValVerticalRef + angleValThreshold) - axisData.x;
				val = mymin(val, angleDoubleScope);
				xErrorMap2Time = map(val, 0, angleDoubleScope, 300, 10);
				if (ptrPage->_itrColums != ptrPage->_listColums.begin()) {
					if(stateTimeOut2(xErrorMap2Time)) {
						ptrPage->_itrColums--;
						indexColums--;
						changeCartoonColums = true;
						buttons = BUTTON_F_SHORT;
				}
			}
		}

#endif
			const uint8_t lastExitCnt = 1;
			static uint8_t lastExit = lastExitCnt;

			if (stateTimeOut() && (buttons == BUTTON_OK_LONG/* || moveDetected)*/)) {
#if DoubleMenu	/*仅支持二级菜单*/
				ptrPage = homePage;
#else			/*支持三级及以上菜单*/
				ptrPage = ptrPageList.back();	//此时ptrPage才是前一个Page*
				if(ptrPage != ptrPageList.front()) //防止越界删除，实测有效
					ptrPageList.pop_back();	//先删除尾端Page
#endif
				resetPageIndex(false);
			}

#if 1
			// 矩形动画偏移
			valIndex = *(indexColums.val);
			if( valIndex == 16 && (buttons == BUTTON_B_SHORT || buttons == BUTTON_B_LONG) && swapCartoonFreshColumsRec) {
				swapCartoonFreshColumsRec = 0;
				cartoonFreshColums(1);
				changeCartoonColums = false;
			}
			else if(valIndex == 0 && (buttons == BUTTON_F_SHORT|| buttons == BUTTON_F_LONG) && !swapCartoonFreshColumsRec) {
				swapCartoonFreshColumsRec = 1;
				cartoonFreshColums(0);
				changeCartoonColums = false;
			}
			//buttons = 0;
			// 文字动画偏移
			std::list<Colum*>::iterator itrCol = ptrPage->_itrColums;
			if(changeCartoonColums && (itrCol != ptrPage->_listColums.begin() || itrCol != ptrPage->_listColums.end()))
			{
				if(valIndex == 16 && (buttons == BUTTON_B_SHORT|| buttons == BUTTON_B_LONG) && !swapCartoonFreshColumsRec) {
					cartoonFreshFonts(1);
				}
				else if(valIndex == 0 && (buttons == BUTTON_F_SHORT|| buttons == BUTTON_F_LONG) && swapCartoonFreshColumsRec) {
					cartoonFreshFonts(0);
				}
			}
#endif

			//taskENTER_CRITICAL();
			ptrPage->drawColums();
			//taskEXIT_CRITICAL();
			u8g2.sendBuffer();
			GUIDelay();
#if 1
			//到最父级的homePage页才退出本函数块
			if(ptrPage == ptrPageList.front() &&
					stateTimeOut() &&
					(buttons == BUTTON_OK_LONG/* || moveDetected)*/))
			{
				--lastExit;
				if(!lastExit )
				{
					lastExit = lastExitCnt;
					moveDetected = false;
					break;
				}
			}
#endif
			resetWatchdog();
		}
	}

	static void columValAdjust(Colum *ptrColum) {
		AutoValue *ptrAutoValue = ptrColum->ptrAutoValue;
		uint32_t lastChange = xTaskGetTickCount();
		//入口处执行函数指针的不用执行else部分
		if (ptrColum->str == nullptr)
			return;
		if (ptrColum->funLoc == LOC_ENTER) {
			ptrColum->funPtr();
		} else {

			for (;;) {
				//u8g2.clearBuffer();	//保留上级for循环刷新的buffer，本节只刷新值
				buttons = getButtonState();
				if (buttons)
					lastChange = xTaskGetTickCount();
				AutoValue::buttonState = buttons;

				//taskENTER_CRITICAL();	//临界段代码，防止绘图时被打乱出现错位
				switch (buttons) {
				case BUTTON_B_LONG:
				case BUTTON_B_SHORT:
					(*ptrAutoValue)--;//AutoValue::operator--()在内部判断buttons长按短按
					if (ptrColum->funLoc == LOC_CHANGE)
						ptrColum->funPtr();
					break;
				case BUTTON_F_LONG:
				case BUTTON_F_SHORT:
					(*ptrAutoValue)++;
					if (ptrColum->funLoc == LOC_CHANGE)
						ptrColum->funPtr();
					break;
				default:
					break;
				}

				uint16_t y = *(indexColums.val);
				ptrPage->drawColum(ptrColum, y, 1);
				y += 1;
				u8g2.setDrawColor(0);
				u8g2.drawStr(82, y, "*");	//绘制更改标记
				//taskEXIT_CRITICAL();
				u8g2.sendBuffer();

				if ((xTaskGetTickCount() - lastChange > 2000)
						|| (buttons == BUTTON_OK_SHORT)) {
					if (ptrColum->funLoc == LOC_EXTI)
						ptrColum->funPtr();
					if (ptrColum->ptrAutoValue != nullptr) {
						saveSettings();
						resetSettings();
						restoreSettings();
					}
					return;
				}

				GUIDelay();
			}
		}
	}

	//绘制能显示在oled屏幕可见范围内的colums
	void drawColums() {
		//绘制被选中的colum
		drawColum(*_itrColums, *(indexColums.val), 1);
		//绘制被选中colum上面的colum
		for (uint8_t i = indexColums.lower; i < *(indexColums.val);) {
			i += indexColums.shortSteps;
			drawColum(
					*_itrColums
							- (*(indexColums.val) - indexColums.lower)
									/ indexColums.shortSteps
							+ (i / indexColums.shortSteps) - 1,
					i - indexColums.shortSteps, 0);
		}
		//绘制被选中colum下面的colum
		for (uint8_t i = 0; i < indexColums.upper - *(indexColums.val);) {
			i += indexColums.shortSteps;
			drawColum(*_itrColums + (i / indexColums.shortSteps),
					*(indexColums.val) + i, 0);
		}
	}

	//绘制一个colum
	//selected 可能为 0，1，2。 其中2为颜色叠加模式
	void drawColum(Colum *ptrColum, int8_t y, uint8_t selected) {
		//绘制反显矩形
		if(selected != 2) {
			u8g2.setDrawColor(selected);
			u8g2.drawBox(0, y, 128, 16);
			u8g2.setDrawColor(!selected);
		}
		else
		{
			//不要绘制黑色矩形，动画过渡会闪屏
			//u8g2.setDrawColor(!selected);
		    //u8g2.drawBox(0, y, 128, 16);

			u8g2.setDrawColor(selected); //color 2模式
		}

		//绘制栏名称字符,宋体UTF-8
		y += 2;	//偏移字符串y坐标
		if (ptrColum->str != nullptr) {
			u8g2.setFont(u8g2_simsun_9_fntodgironchinese);	//12x12 pixels
			u8g2.drawUTF8(1, y, ptrColum->str);	//打印中文字符，编译器需要支持UTF-8编码，显示的字符串需要存为UTF-8编码
		}

#if 1
		if (ptrColum->ptrAutoValue != nullptr) {
			//绘制栏详情字符
			y -= 1;	//偏移字符串y坐标

			if (!(*ptrColum->ptrAutoValue).valueIsBool()) {
				if(ptrColum->ptrColumVal2Str != nullptr)
				{
					std::map<uint16_t, const char*>::iterator itr = ptrColum->ptrColumVal2Str->find(*(ptrColum->ptrAutoValue)->val);
					u8g2.drawUTF8( 128 -  strlen(itr->second) / 3 /*"中" = 3 "中文" = 6 "中文字" = 9;=*/
							* 12/*12=字体宽度*/ -3 /*边缘偏移*/, y, itr->second);
				}
				else
				{
					// 修改字体为非中文字体
					u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
#if 1
					Page::drawNumber(113 - (ptrColum->ptrAutoValue->places) * 6, y,
							*(ptrColum->ptrAutoValue)->val,
							(ptrColum->ptrAutoValue)->places);
#endif
					if (ptrColum->unit != nullptr)
						u8g2.drawStr(119, y, ptrColum->unit);
				}
			}
			else
			{
				u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
				(*(ptrColum->ptrAutoValue)->val == true) ?
						u8g2.drawStr(111, y, "ON") :
						u8g2.drawStr(103, y, "OFF");
			}
	}
#endif
}

	static void resetPageIndex(bool reset) {
		if (reset) {
			//强制指向第一个栏
			ptrPage->_itrColums = ptrPage->_listColums.begin();
			(*(indexColums.val) = 0);
		} else
			//尾端检查indexColums防止非法访问
			(*(ptrPage->_itrColums) == ptrPage->_listColums.back()) ?
					(*(indexColums.val) = indexColums.upper) :
					(*(indexColums.val) = 0);
	}
	static void drawNumber(uint8_t x, uint8_t y, uint16_t number,
			uint8_t places) {
		char buffer[7] = { 0 };
		//sprintf(buffer, "%06d" , number);
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
		u8g2.drawStr(x, y, buffer + cntFirstNum - (places - cntNum));
		//u8g2.drawUTF8(x, y, buffer + cntFirstNum - (places - cntNum));
	}
	static bool stateTimeOut() {
		uint32_t previousState = xTaskGetTickCount();
		static uint32_t previousStateChange = xTaskGetTickCount();
		if ((previousState - previousStateChange) > 500) {	///	这个500决定向父级菜单递归的阻塞感
			previousStateChange = previousState;
			return true;
		}
		return false;
	}

	static bool stateTimeOut2(uint16_t time = 200) {
		uint32_t previousState = xTaskGetTickCount();
		static uint32_t previousStateChange = xTaskGetTickCount();
		if ((previousState - previousStateChange) > time) {	///	这个200决定向父级菜单递归的阻塞感
			previousStateChange = previousState;
			return true;
		}
		return false;
	}

	static Colum* getColumsSelected() {
		return *ptrPage->_itrColums;
	}

	//求当前_itrColums是当前_listColums的第几个元素，0表示第一个, 测试OK
	static int num_of_itrColum() {
		return *ptrPage->_itrColums - *ptrPage->_listColums.begin();
	}

	//绘制icons，未使用
	void drawIcon() {
		//u8g2.setDrawColor(1);
		//u8g2.drawXBM(90, 0, 32, 32, _Icon);
		;
	}


	/**
	  * @brief  栏条矩形动画偏移函数，仅支持两级菜单栏栏滚动
	  * @param  Dir 滚动方向，0向上，1向下
	  * @param Steps 动画步幅除数：范围{0, 1, 2, 4, 8, 16}，默认为4步幅
	  * @retval None
	  */
	static void cartoonFreshColums(bool Dir, uint8_t Steps = 4)
	{
		uint8_t *ptrBuffer = u8g2.getBufferPtr();	//得到U8g2的屏幕显示缓冲区地址，当前总缓冲区大小为 128 x 8 x 4 bit
		uint8_t *ptrRecFront;
		uint8_t *ptrRecBack;
		if(Dir) {
			ptrRecFront = ptrBuffer;
			ptrRecBack = ptrBuffer + 128 * 2;
		}
		else {
			ptrRecFront = ptrBuffer + 128 * 1;
			ptrRecBack = ptrBuffer + 128 * 3;

		}

		uint8_t cntShowFrame = 0;
		uint8_t Bit;

		for (uint8_t cntY = 0; cntY < 2; cntY++)
		{
			resetWatchdog();
			//加128，屏幕上表现为自动换行
			int cnt = cntY * 128;
			if(Dir) {
				ptrRecFront += cnt;
				ptrRecBack += cnt;
			}else {
				ptrRecFront -= cnt;
				ptrRecBack -= cnt;
			}
			uint8_t cntBitCrtl;
			for (uint8_t cntBit = 0; cntBit < 8; cntBit++)
			{
				cntBitCrtl = 7 - cntBit;
				if(Dir)
					Bit = cntBit;
				else
					Bit = cntBitCrtl;

				uint8_t cntX;
				for(cntX = 0; cntX < 128; cntX ++)
				{
					bitWrite(*ptrRecFront, Bit, !bitRead(*ptrRecFront, Bit));
					bitWrite(*ptrRecBack, Bit, !bitRead(*ptrRecBack, Bit));
					ptrRecFront++;
					ptrRecBack++;
				}
				ptrRecFront -= 128;
				ptrRecBack -= 128;
				//刷新buffer即24帧，约2秒
				cntShowFrame = (cntShowFrame + 1) % Steps;
				if(!cntShowFrame) {
					u8g2.sendBuffer();
					osDelay(10);
				}
			}
		}
	}
	/**
	  * @brief  栏条字体动画偏移函数，仅支持两级菜单栏栏滚动
	  * @param  Dir 滚动方向，0向上，1向下
	  * @param Steps 动画步幅除数：范围{0, 1, 2, 4, 8, 16}，默认为4
	  * @retval None
	  */
	static void cartoonFreshFonts(bool Dir = 0, uint8_t Steps = 4)
	{
		u8g2.setDrawColor(1);
		u8g2.setFontMode(1);

		//若valIndex=16，则 _itrColums相比当前显示选中的colum已经向下迭代一位
		//若valIndex=0，则 _itrColums相比当前显示选中的colum已经向上迭代一位
		std::list<Colum*>::iterator itrColums = ptrPage->_itrColums;

		if(valIndex == 16) {
			for(uint8_t cntY = 0; cntY < 16;) {
				itrColums = ptrPage->_itrColums;
				--itrColums;
				--itrColums;
				//绘制底图矩形
				//u8g2.clearBuffer();
				u8g2.setDrawColor(1);
				u8g2.drawBox(0, 16, 128, 16);
				u8g2.setDrawColor(0);
				u8g2.drawBox(0, 0, 128, 16);

				u8g2.setFontPosBottom();
				int8_t cntYCase = 15 - cntY -2;
				if(cntYCase >= 0)
					ptrPage->drawColum(*itrColums, cntYCase, 2);
				++itrColums;
				u8g2.setFontPosTop();	//还原字体上对其
				cntYCase = 15 - cntY;
				if(cntYCase >= 0)
					ptrPage->drawColum(*itrColums, cntYCase, 2);
				++itrColums;
				ptrPage->drawColum(*itrColums, 31 - cntY, 2);

				u8g2.sendBuffer();
				osDelay(10);

				cntY += Steps;
			}
		}
		else if(valIndex == 0) {
			for(uint8_t cntY = 0; cntY < 16;) {
				itrColums = ptrPage->_itrColums;
				++itrColums;
				++itrColums;
				//绘制底图矩形
				//u8g2.clearBuffer();
				u8g2.setDrawColor(1);
				u8g2.drawBox(0, 0, 128, 16);
				u8g2.setDrawColor(0);
				u8g2.drawBox(0, 16, 128, 16);

				u8g2.setFontPosTop();	//还原字体上对其
				int8_t cntYCase = 15 + cntY + 1;
				if(cntYCase <= 31)
					ptrPage->drawColum(*itrColums, cntYCase, 2);
				--itrColums;
				ptrPage->drawColum(*itrColums, cntY + 1, 2);
				--itrColums;
				u8g2.setFontPosBottom();
				cntYCase = cntY -2;
				if(cntYCase >= 0)
					ptrPage->drawColum(*itrColums, cntYCase, 2);
				u8g2.setFontPosTop();
				u8g2.sendBuffer();
				osDelay(10);

				cntY += Steps;
			}
		}
		u8g2.setFontMode(0);
	}
	std::list<Colum*> _listColums;
	static AutoValue indexColums;	//用于索引当前选中的colum在oled上的y坐标位置
	static Page *ptrPage;
	static Page *ptrPagePrev;
	static Page *homePage;
	static std::list<Page *> ptrPageList;
	static bool timeOut;			//级联菜单共用返回上级的停顿感延时的标记
	static uint8_t valIndex;		//用于临时储存Page::indexColums的val值
	//static uint8_t buttons;				//按钮状态，debug用
private:
	std::list<Colum*>::iterator _itrColums;
	//const uint8_t *_Icon;
	Page *_nextPage;
	Page *_prevPage;
	uint8_t _numColums;
};

extern Page pageHomePage;
extern std::vector<uint16_t> *ptrAutoValueLanguageVals;



#endif
#endif
