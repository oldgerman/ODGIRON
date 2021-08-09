#include "BSP.h"
#include "BSP_Power.h"
#include "Model_Config.h"
#include "Settings.h"

#include "fusbpd.h"
#include "int_n.h"
#include "policy_engine.h"
bool FUSB302_present = false;	//标记是FUSB302B否存在

void power_check() {
#ifdef POW_PD
#if 1
  if (FUSB302_present) {
    // Cant start QC until either PD works or fails
    if (PolicyEngine::setupCompleteOrTimedOut() == false) {
      return;
    }
    if (PolicyEngine::pdHasNegotiated()) {
      return;
    }
  }
#endif
#endif
#ifdef POW_QC
  QC_resync();
#endif
}


/**
 * 检测i2c总线上是否有FUSB302B
 */
uint8_t usb_pd_detect() {
#ifdef POW_PD
#if 1
  FUSB302_present = fusb302_detect();
  return FUSB302_present;
#else
	return true; //oldgerman强制测试用
#endif
#endif
  return false;
}


/**
 * 判定供电方式
 * true： DC供电
 * false 协议供电
 */
bool getIsPoweredByDCIN() {
#ifdef MODEL_TS80
  return false;
#endif

#ifdef MODEL_TS80P
  return false;
#endif

#ifdef MODEL_TS100
  return true;
#endif
}
