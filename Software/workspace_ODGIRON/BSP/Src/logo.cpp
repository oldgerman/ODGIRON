/*
 * logo.c
 *
 *  Created on: 29 May 2020
 *      Author: Ralim
 */

#include "BSP.h"
#include "Threads.hpp"
#include "font24x32numbers.h"
#include "Page.hpp"
//#include "OLED.hpp"

//static uint8_t logo_page[1024] __attribute__((section(".logo_page")));

// Logo header signature.
// 徽标标头签名。
#define LOGO_HEADER_VALUE 0xF00DAA55

uint8_t showBootLogoIfavailable() {
	// Do not show logo data if signature is not found.
	//if (LOGO_HEADER_VALUE != *(reinterpret_cast<const uint32_t *>(logo_page))) {
	//  return 0;
	//}
	//return 0;

	//OLED::drawAreaSwapped(0, 0, 96, 16, (uint8_t *)(logo_page + 4));
#if 0
	OLED::setFont(0);

	OLED::setCursor(0, 0);
	OLED::print("ODGIRON");

	OLED::setFont(1);
	OLED::setCursor(0, 16);
	OLED::print("ironOS-based PD T12\nversion: 1.0");

	OLED::refresh();
#endif

	u8g2.drawXBM(0, 0, 128, 32, solderingPage);
	return 1;
}
