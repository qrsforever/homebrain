/***************************************************************************
 *  LFHelper.cpp -
 *
 *  Created: 2019-07-17 17:02:54
 *
 *  Copyright QRS
 ****************************************************************************/

#include "LFHelper.h"
#include "DBTables.h"
#include "LFDatabase.h"
#include "LFTypes.h"
#include "Log.h"

extern "C" int ota_get_version(char *local, char* newv, int len);
extern "C" int ota_upgrade(char *buf, int len);
extern "C" int ota_init(void);
extern "C" int upload_log(char *logfilename,  char* elinkid, char *logid);

namespace HB {

const char* LFBuildVersion()
{/*{{{*/
    return LF_BUILD_VERSION;
}/*}}}*/

const char* LFBuildDatetime()
{/*{{{*/
    return LF_BUILD_DATETIME;
}/*}}}*/

LarfeClient& deviceManager()
{/*{{{*/
    static LarfeClient* sLFClient = 0;
    if (!sLFClient)
        sLFClient = new LarfeClient();
    return *sLFClient;
}/*}}}*/

int LFOTAInit()
{/*{{{*/
    return ota_init();
}/*}}}*/

// -1: error 0: no new version 1: exist new version
int LFOTAGetVersion(std::string& curVersion, std::string& newVersion)
{/*{{{*/
    char cur_ver[128] = { 0 };
    char new_ver[128] = { 0 };
    int ret = ota_get_version(cur_ver, new_ver, 127);
    if (-1 != ret) {
        curVersion.assign(cur_ver);
        if (ret == 1)
            newVersion.assign(new_ver);
    }
    LOGD("ret[%d]\n", ret);
    return ret;
}/*}}}*/

// -1: error 0: success
int LFOTAUpgrade(const std::string& targetVersion)
{/*{{{*/
    char res[128] = { 0 };
    int ret = ota_upgrade(res, 127);
    if (-1 != ret)
        LOGD("target[%s] res[%s]\n", targetVersion.c_str(), res);
    return ret;
}/*}}}*/

int LFLogUpload(std::string logfile, std::string elinkId, std::string key, std::string logId)
{/*{{{*/
    LOGA("(%s, %s, %s, %s)\n", logfile.c_str(), elinkId.c_str(), key.c_str(), logId.c_str());
    if (logfile.empty() || elinkId.empty())
        return -1;
    return upload_log((char*)logfile.c_str(), (char*)elinkId.c_str(), (char*)logId.c_str());
}/*}}}*/

void LFDumpInfo()
{/*{{{*/
    /* Bulild info */
    LOGA("\n\n\n===================BEGIN=======================\n");

    LOGA("BuildVersion[%s] BuildDatetime[%s]\n", LFBuildVersion(), LFBuildDatetime());

    /* Dump database */
    mainDB().dump(LFGlobalInfo());
    mainDB().dump(LFLogInfo());
    mainDB().dump(LFDeviceInfo());

    /* Elink */
    LOGA("ELINK_CLOUD_HOST[%s] ELINK_EMQ_HOST[%s]\n", ELINK_CLOUD_HOST, ELINK_EMQ_HOST);

    /* Other */
    LOGA("UPGRADE_CHECK_INTERVAL[%d]\n", UPGRADE_CHECK_INTERVAL);

    LOGA("\n====================END========================\n\n\n");
}/*}}}*/

}
