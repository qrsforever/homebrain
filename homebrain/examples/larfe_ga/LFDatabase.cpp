/***************************************************************************
 *  LFDatabase.cpp -
 *
 *  Created: 2019-07-11 11:05:07
 *
 *  Copyright QRS
 ****************************************************************************/

#include "LFDatabase.h"
#include "SQLiteLog.h"
#include "DBTables.h"
#include "MessageTypes.h"
#include "MainHandler.h"

using namespace UTILS;

namespace HB {

static LFDatabase *gDB = 0;

LFDatabase::LFDatabase() : mDBPath("lf.db"), mAutoCloseInterval(15000), mDB(0)
{
}

LFDatabase::~LFDatabase()
{

}

void LFDatabase::init(const std::string &path, int interval)
{/*{{{*/
    if (path.empty())
        mDBPath = "./lf.db";
    else
        mDBPath = path + "/lf.db";
    mAutoCloseInterval = interval;
}/*}}}*/

bool LFDatabase::open()
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

bool LFDatabase::close()
{/*{{{*/
    SQL_LOGD("database[%s]\n", mDBPath.c_str());

    if (mDB)
        delete mDB;
    mDB = 0;

    return true;
}/*}}}*/

/*{{{ Global Table */
template<>
bool LFDatabase::updateOrInsert<LFGlobalInfo>(const LFGlobalInfo &info)
{
    SQL_LOGI("update larfe table\n");

    if (!open())
        return false;

    LFGlobalTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool LFDatabase::deleteBy<LFGlobalInfo>(const LFGlobalInfo &info)
{
    SQL_LOGI("delete larfe table\n");

    if (!open())
        return false;

    LFGlobalTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool LFDatabase::queryBy<LFGlobalInfo>(std::vector<LFGlobalInfo> &infos, const LFGlobalInfo &info)
{
    SQL_LOGI("query larfe table\n");

    if (!open())
        return false;

    LFGlobalTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
bool LFDatabase::queryBy<LFGlobalInfo>(std::vector<LFGlobalInfo> &infos, const std::string &key)
{
    SQL_LOGD("query larfe table\n");

    if (!open())
        return false;

    LFGlobalTable tab(*mDB);

    return tab.queryBy(infos, LFGlobalInfo(key), "");
}

template <>
void LFDatabase::dump<LFGlobalInfo>(const LFGlobalInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    LFGlobalTable tab(*mDB);

    return tab.dump();
}

/*}}}*/

/*{{{ Log Table */
template<>
bool LFDatabase::updateOrInsert<LFLogInfo>(const LFLogInfo &info)
{
    SQL_LOGI("update log table\n");

    if (!open())
        return false;

    LFLogTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool LFDatabase::deleteBy<LFLogInfo>(const LFLogInfo &info)
{
    SQL_LOGI("delete log table\n");

    if (!open())
        return false;

    LFLogTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool LFDatabase::queryBy<LFLogInfo>(std::vector<LFLogInfo> &infos, const LFLogInfo &info)
{
    SQL_LOGI("query log table\n");

    if (!open())
        return false;

    LFLogTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
void LFDatabase::dump<LFLogInfo>(const LFLogInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    LFLogTable tab(*mDB);

    return tab.dump();
}
/*}}}*/

/*{{{ Device Table */
template<>
bool LFDatabase::updateOrInsert<LFDeviceInfo>(const LFDeviceInfo &info)
{
    SQL_LOGI("update device table\n");

    if (!open())
        return false;

    LFDeviceTable tab(*mDB);

    bool res = tab.updateOrInsert(info);
    if (res) {
        mainHandler().removeMessages(MT_DB, DB_TABLE_UPDATE, tab.tableType());
        mainHandler().sendMessageDelayed(
            mainHandler().obtainMessage(MT_DB, DB_TABLE_UPDATE, tab.tableType()), 5000);
    }
    return res;
}

template<>
bool LFDatabase::deleteBy<LFDeviceInfo>(const LFDeviceInfo &info)
{
    SQL_LOGI("delete device table\n");

    if (!open())
        return false;

    LFDeviceTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool LFDatabase::queryBy<LFDeviceInfo>(std::vector<LFDeviceInfo> &infos, const LFDeviceInfo &info)
{
    SQL_LOGI("query device table\n");

    if (!open())
        return false;

    LFDeviceTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
bool LFDatabase::queryBy<LFDeviceInfo>(std::vector<LFDeviceInfo> &infos, const std::string &key)
{
    SQL_LOGD("query device table\n");

    if (!open())
        return false;

    LFDeviceTable tab(*mDB);

    return tab.queryBy(infos, LFDeviceInfo(key), "");
}

template <>
void LFDatabase::dump<LFDeviceInfo>(const LFDeviceInfo &info)
{
    SQL_LOGI("dump larfe table\n");

    if (!open())
        return;
    LFDeviceTable tab(*mDB);

    return tab.dump();
}

/*}}}*/

LFDatabase& mainDB()
{
    if (0 == gDB)
        gDB = new LFDatabase();
    return *gDB;
}

} /* namespace HB */
