/***************************************************************************
 *  KKHelper.cpp -
 *
 *  Created: 2019-09-27
 *
 *  Copyright QRS
 ****************************************************************************/

#include "KKHelper.h"
#include "DBTables.h"
#include "KKDatabase.h"
#include "KKTypes.h"
#include "Log.h"

namespace HB {

const char* KKBuildVersion()
{/*{{{*/
    return KK_BUILD_VERSION;
}/*}}}*/

const char* KKBuildDatetime()
{/*{{{*/
    return KK_BUILD_DATETIME;
}/*}}}*/

KKClient& deviceManager()
{/*{{{*/
    static KKClient* sKKClient = 0;
    if (!sKKClient)
        sKKClient = new KKClient();
    return *sKKClient;
}/*}}}*/

void KKDumpInfo()
{/*{{{*/
    /* Bulild info */
    LOGA("\n\n\n===================BEGIN=======================\n");

    LOGA("BuildVersion[%s] BuildDatetime[%s]\n", KKBuildVersion(), KKBuildDatetime());

    /* Dump database */
    mainDB().dump(KKGlobalInfo());
    mainDB().dump(KKLogInfo());
    mainDB().dump(KKDeviceInfo());

    /* Elink */
    LOGA("ELINK_CLOUD_HOST[%s] ELINK_EMQ_HOST[%s]\n", ELINK_CLOUD_HOST, ELINK_EMQ_HOST);

    /* Other */
    LOGA("UPGRADE_CHECK_INTERVAL[%d]\n", UPGRADE_CHECK_INTERVAL);

    LOGA("\n====================END========================\n\n\n");
}/*}}}*/

}
