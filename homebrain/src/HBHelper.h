/***************************************************************************
 *  HBHelper.h -
 *
 *  Created: 2018-08-06 17:53:43
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __HBHelper_H__
#define __HBHelper_H__

#ifdef SIM_SUITE
#include "TempSimulateSuite.h" /* TODO only test */
#else
#include "HBDeviceManager.h"
#include "HBCloudManager.h"
using namespace OIC::Service::HB;
#endif

#ifdef __cplusplus

namespace HB {

HBCloudManager& cloudManager();

HBDeviceManager& deviceManager();

void deviceProfileCheckAndUpdate(const char *path);

int lightSystemCall(int what, int argc, char *args[]);

int startBridge(const char* bridgeType, const char *bridgeId, const char *accessKey);
int killBridge(const char *bridgeId);

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __HBHelper_H__ */
