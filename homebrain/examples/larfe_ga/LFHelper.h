/***************************************************************************
 *  LFHelper.h -
 *
 *  Created: 2019-07-17 17:02:58
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LFHelper_H__
#define __LFHelper_H__

#include "LarfeClient.h"
#include <string>

#ifdef __cplusplus

namespace HB {

const char* LFBuildVersion();
const char* LFBuildDatetime();

LarfeClient& deviceManager();

int LFOTAInit();
int LFOTAGetVersion(std::string& curVersion, std::string& newVersion);
int LFOTAUpgrade(const std::string& targetVersion);
int LFLogUpload(std::string logfile, std::string elinkId, std::string key, std::string logId);
void LFDumpInfo();

}

#endif /* __cplusplus */

#endif /* __LFHelper_H__ */
