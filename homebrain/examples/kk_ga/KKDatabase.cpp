/***************************************************************************
 *  KKDatabase.cpp -
 *
 *  Created: 2019-09-27
 *
 *  Copyright QRS
 ****************************************************************************/

#include "KKDatabase.h"
#include "SQLiteLog.h"
#include "DBTables.h"
#include "MessageTypes.h"
#include "MainHandler.h"

using namespace UTILS;

namespace HB {

static KKDatabase *gDB = 0;

KKDatabase::KKDatabase() : mDBPath("lf.db"), mAutoCloseInterval(15000), mDB(0)
{
}

KKDatabase::~KKDatabase()
{

}

void KKDatabase::init(const std::string &path, int interval)
{/*{{{*/
    if (path.empty())
        mDBPath = "./lf.db";
    else
        mDBPath = path + "/lf.db";
    mAutoCloseInterval = interval;
}/*}}}*/

bool KKDatabase::open()
{/*{{{*/
    /* auto close db after AUTO_CLOSE_DB_TIME */
    mainHandler().removeMessages(MT_DB, DB_CLOSE, 0);
    mainHandler().sendMessageDelayed(
        mainHandler().obtainMessage(MT_DB, DB_CLOSE, 0), mAutoCloseInterval);
    if (isOpen())
        return true;

    SQL_LOGD("database[%s]\n", mDBPath.c_str());

    mDB = new SQLiteDatabase();
    if (!mDB->open(mDBPath.c_str()))
        return false;
    return true;
}/*}}}*/

bool KKDatabase::close()
{/*{{{*/
    SQL_LOGD("database[%s]\n", mDBPath.c_str());

    if (mDB)
        delete mDB;
    mDB = 0;

    return true;
}/*}}}*/

/*{{{ Global Table */
template<>
bool KKDatabase::updateOrInsert<KKGlobalInfo>(const KKGlobalInfo &info)
{
    SQL_LOGI("update larfe table\n");

    if (!open())
        return false;

    KKGlobalTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool KKDatabase::deleteBy<KKGlobalInfo>(const KKGlobalInfo &info)
{
    SQL_LOGI("delete larfe table\n");

    if (!open())
        return false;

    KKGlobalTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool KKDatabase::queryBy<KKGlobalInfo>(std::vector<KKGlobalInfo> &infos, const KKGlobalInfo &info)
{
    SQL_LOGI("query larfe table\n");

    if (!open())
        return false;

    KKGlobalTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
bool KKDatabase::queryBy<KKGlobalInfo>(std::vector<KKGlobalInfo> &infos, const std::string &key)
{
    SQL_LOGD("query larfe table\n");

    if (!open())
        return false;

    KKGlobalTable tab(*mDB);

    return tab.queryBy(infos, KKGlobalInfo(key), "");
}

template <>
void KKDatabase::dump<KKGlobalInfo>(const KKGlobalInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    KKGlobalTable tab(*mDB);

    return tab.dump();
}

/*}}}*/

/*{{{ Log Table */
template<>
bool KKDatabase::updateOrInsert<KKLogInfo>(const KKLogInfo &info)
{
    SQL_LOGI("update log table\n");

    if (!open())
        return false;

    KKLogTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool KKDatabase::deleteBy<KKLogInfo>(const KKLogInfo &info)
{
    SQL_LOGI("delete log table\n");

    if (!open())
        return false;

    KKLogTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool KKDatabase::queryBy<KKLogInfo>(std::vector<KKLogInfo> &infos, const KKLogInfo &info)
{
    SQL_LOGI("query log table\n");

    if (!open())
        return false;

    KKLogTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
void KKDatabase::dump<KKLogInfo>(const KKLogInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    KKLogTable tab(*mDB);

    return tab.dump();
}
/*}}}*/

/*{{{ Device Table */
template<>
bool KKDatabase::updateOrInsert<KKDeviceInfo>(const KKDeviceInfo &info)
{
    SQL_LOGI("update device table\n");

    if (!open())
        return false;

    KKDeviceTable tab(*mDB);

    bool res = tab.updateOrInsert(info);
    if (res) {
        mainHandler().removeMessages(MT_DB, DB_TABLE_UPDATE, tab.tableType());
        mainHandler().sendMessageDelayed(
            mainHandler().obtainMessage(MT_DB, DB_TABLE_UPDATE, tab.tableType()), 5000);
    }
    return res;
}

template<>
bool KKDatabase::deleteBy<KKDeviceInfo>(const KKDeviceInfo &info)
{
    SQL_LOGI("delete device table\n");

    if (!open())
        return false;

    KKDeviceTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool KKDatabase::queryBy<KKDeviceInfo>(std::vector<KKDeviceInfo> &infos, const KKDeviceInfo &info)
{
    SQL_LOGI("query device table\n");

    if (!open())
        return false;

    KKDeviceTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
bool KKDatabase::queryBy<KKDeviceInfo>(std::vector<KKDeviceInfo> &infos, const std::string &key)
{
    SQL_LOGD("query device table\n");

    if (!open())
        return false;

    KKDeviceTable tab(*mDB);

    return tab.queryBy(infos, KKDeviceInfo(key), "");
}

template <>
void KKDatabase::dump<KKDeviceInfo>(const KKDeviceInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    KKDeviceTable tab(*mDB);

    return tab.dump();
}

/*}}}*/

KKDatabase& mainDB()
{
    if (0 == gDB)
        gDB = new KKDatabase();
    return *gDB;
}

} /* namespace HB */
