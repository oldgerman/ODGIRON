/*
 * fusbpd.cpp
 *
 *  Created on: 13 Jun 2020
 *      Author: Ralim
 */
#include "Model_Config.h"
#ifdef POW_PD
#include "fusbpd.h"
#include "BSP.h"
#include "fusb302b.h"
#include "int_n.h"
#include "policy_engine.h"
#include "protocol_rx.h"
#include "protocol_tx.h"
#include <fusbpd.h>
#include <pd.h>

void fusb302_start_processing() {
  /* Initialize the FUSB302B */
  if (fusb_setup()) {
	  //创建4个Task
    PolicyEngine::init();		//策略引擎线程
    ProtocolTransmit::init();	//Tx线程
    ProtocolReceive::init();	//Rx线程
    InterruptHandler::init();	//FUSB302引脚中断线程
  }
}
#endif
