/***************************************************************************
 *  RuleEngineLog.cpp - Rule Engine Log impl
 *
 *  Created: 2018-07-18 15:39:46
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBDeviceManagerLog.h"

int gDeviceManagerModuleLevel = LOG_LEVEL_DEBUG;

namespace HB {

static LogModule HBDeviceManagerModule("device-manager", gDeviceManagerModuleLevel);

} /* namespace HB */
