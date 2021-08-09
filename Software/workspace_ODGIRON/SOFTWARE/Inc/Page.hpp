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

#ifdef __cplusplus
#include <algorithm>
extern char * ttstr;
extern int8_t xxxp;
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
		ptrPageList.push_back(ptrPage);
		for (;;) {
			//u8g2.clearBuffer();
			//u8g2.clearBuffer();不要随便用clearbuffer，会导致显示乱码，u8g2是命令也在buffer里多次刷新的
			buttons = getButtonState();
			switch (buttons) {
			case BUTTON_B_SHORT:
				if (*(ptrPage->_itrColums) != ptrPage->_listColums.back()) {
					ptrPage->_itrColums++;
					indexColums++;
				}
				break;
			case BUTTON_B_LONG:
#if 0
				if (stateTimeOut() && *(ptrPage->_itrColums) != ptrPage->_listColums.back()) {
					ptrPage->_itrColums++;
					indexColums++;
				}
#endif
				break;
			case BUTTON_F_SHORT:
				if (ptrPage->_itrColums != ptrPage->_listColums.begin()) {
					ptrPage->_itrColums--;
					indexColums--;
				}
				break;
			case BUTTON_F_LONG:
#if 0
				if (stateTimeOut() && ptrPage->_itrColums != ptrPage->_listColums.begin()) {
					ptrPage->_itrColums--;
					indexColums--;
				}
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
				/*仅支持三级及以上菜单*/
				if ((*ptrPage->_itrColums)->nextPage != nullptr)
				{
					if(firshInflashPage) {
						firshInflashPage = 0;
					}else{
						ptrPageList.push_back(ptrPage);
					}
						ptrPage = (*ptrPage->_itrColums)->nextPage; //当前页面指针指向当前页面指针指向的Colum的下级菜单
						resetPageIndex(false);
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
			const uint8_t lastExitCnt = 1;
			static uint8_t lastExit = lastExitCnt;
			if (stateTimeOut() && (buttons == BUTTON_B_LONG/* || moveDetected)*/)) {
#if DoubleMenu
				ptrPage = homePage;
#else
				ptrPage = ptrPageList.back();	//此时ptrPage才是前一个Page*
				if(ptrPage != ptrPageList.front()) //防止越界删除，实测有效
					ptrPageList.pop_back();	//先删除尾端Page
#endif
				resetPageIndex(false);
			}

			ptrPage->drawColums();
			GUIDelay();	//绘制后不要立即sendBuffer，可能有乱码，需要先GUIdelay()再sendBuffer
			u8g2.sendBuffer();
			resetWatchdog();
			if(ptrPage == ptrPageList.front() && stateTimeOut() && (buttons == BUTTON_B_LONG/* || moveDetected)*/)) //到最父级的homePage页才退出本函数块
			{
				--lastExit;
				if(!lastExit )
				{
					lastExit = lastExitCnt;
					moveDetected = false;
					break;
				}
			}
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
	void drawColum(Colum *ptrColum, uint8_t y, bool selected) {
		//绘制反显矩形
		u8g2.setDrawColor(selected);
		u8g2.drawBox(0, y, 128, 16);
		u8g2.setDrawColor(!selected);

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
#if 1
				std::map<uint16_t, const char*>::iterator itr = ptrColum->ptrColumVal2Str->find(*(ptrColum->ptrAutoValue)->val);
				xxxp = strlen(itr->second) / 3;	//"中" = 3 "中文" = 6 "中文字" = 9;
				u8g2.drawUTF8( 128 -  xxxp* 12/*12=字体宽度*/ -3 /*边缘偏移*/, y, itr->second);
#else
				std::map<uint16_t, const char*> mmp = *(ptrColum->ptrColumVal2Str);
				std::map<uint16_t, const char*>::iterator itrxxx = mmp.find(*(ptrColum->ptrAutoValue)->val);

				if (itrxxx != mmp.end())
				{
					const size_t n = strlen(itrxxx->second);   // excludes null terminator
					ttstr = new char[n + 1]{};  // {} zero initializes array
					strcpy(ttstr, itrxxx->second);

					 //此时字体仍中文字体： u8g2_simsun_9_fntodgironchinese
					u8g2.drawUTF8( 98/*113 - (ptrColum->ptrAutoValue->places) * 6*/, y, ttstr);
				}
#endif
				}
				else
				{
					// 修改字体为非中文字体
					u8g2.setFont(u8g2_font_unifont_tr);	//10x7 pixels
					Page::drawNumber(113 - (ptrColum->ptrAutoValue->places) * 6, y,
							*(ptrColum->ptrAutoValue)->val,
							(ptrColum->ptrAutoValue)->places);
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
#endif
	}
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
	static Colum* getColumsSelected() {
		return *ptrPage->_itrColums;
	}
	//绘制icons，未使用
	void drawIcon() {
		//u8g2.setDrawColor(1);
		//u8g2.drawXBM(90, 0, 32, 32, _Icon);
		;
	}
	//static const uint8_t *font = u8g2_simsun_9_fntodgironchinese;

	std::list<Colum*> _listColums;
	static AutoValue indexColums;	//用于索引当前选中的colum在oled上的y坐标位置
	static Page *ptrPage;
	static Page *ptrPagePrev;
	static Page *homePage;
	static std::list<Page *> ptrPageList;
	static bool timeOut;	//级联菜单共用返回上级的停顿感延时的标记
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
