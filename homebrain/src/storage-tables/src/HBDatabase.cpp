/***************************************************************************
 *  HBDatabase.cpp - HomeBrain Database
 *
 *  Created: 2018-11-12 14:11:55
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBDatabase.h"
#include "HBGlobalTable.h"
#include "Log.h"
#include "MessageTypes.h"
#include "MainPublicHandler.h"

#include "RuleEngineTable.h"
#include "DeviceProfileTable.h"
#include "GatewayConnectTable.h"

using namespace UTILS;

namespace HB {

static HBDatabase *gDB = 0;

HBDatabase::HBDatabase()
{

}

HBDatabase::~HBDatabase()
{

}

void HBDatabase::init(const std::string &path, int interval)
{
    if (path.empty())
        mDBPath = "./engine.db";
    else
        mDBPath = path + "/engine.db";
    mAutoCloseInterval = interval;
}

bool HBDatabase::open()
{
    /* auto close db after AUTO_CLOSE_DB_TIME */
    mainHandler().removeMessages(MT_DB, DB_CLOSE, 0);
    mainHandler().sendMessageDelayed(
        mainHandler().obtainMessage(MT_DB, DB_CLOSE, 0), mAutoCloseInterval);
    if (isOpen())
        return true;

    LOGD("database[%s]\n", mDBPath.c_str());

    mDB = new SQLiteDatabase();
    if (!mDB->open(mDBPath.c_str()))
        return false;
    return true;
}

bool HBDatabase::close()
{
    LOGD("database[%s]\n", mDBPath.c_str());

    if (mDB)
        delete mDB;
    mDB = 0;

    return true;
}

/*{{{ Log Table */
template<>
bool HBDatabase::updateOrInsert<LogTableInfo>(const LogTableInfo &info)
{
    LOGD("update log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<LogTableInfo>(const LogTableInfo &info)
{
    LOGD("update log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<LogTableInfo>(std::vector<LogTableInfo> &infos, const LogTableInfo &info)
{
    LOGD("query log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

/*}}}*/

/*{{{ Device Profile Table */
template<>
bool HBDatabase::updateOrInsert<DeviceTableInfo>(const DeviceTableInfo &info)
{
    LOGD("update device table\n");

    if (!open())
        return false;

    DeviceProfileTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<DeviceTableInfo>(const DeviceTableInfo &info)
{
    LOGD("delete device table\n");

    if (!open())
        return false;

    DeviceProfileTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<DeviceTableInfo>(std::vector<DeviceTableInfo> &infos, const DeviceTableInfo &info)
{
    LOGD("query device table\n");

    if (!open())
        return false;

    DeviceProfileTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

/*}}}*/

/*{{{ Rule Table */
template<>
bool HBDatabase::updateOrInsert<RuleTableInfo>(const RuleTableInfo &info)
{
    LOGD("update rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<RuleTableInfo>(const RuleTableInfo &info)
{
    LOGD("delete rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<RuleTableInfo>(std::vector<RuleTableInfo> &infos, const RuleTableInfo &filter)
{
    LOGD("query rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<RuleTableInfo>(std::vector<RuleTableInfo> &infos, const std::string &key)
{
    LOGD("query rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.queryBy(infos, RuleTableInfo(key), "");
}

/*}}}*/

/*{{{ Gateway Table */
template<>
bool HBDatabase::updateOrInsert<GatewayTableInfo>(const GatewayTableInfo &info)
{
    LOGD("update gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<GatewayTableInfo>(const GatewayTableInfo &info)
{
    LOGD("delete gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<GatewayTableInfo>(std::vector<GatewayTableInfo> &infos, const GatewayTableInfo &filter)
{
    LOGD("query gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<GatewayTableInfo>(std::vector<GatewayTableInfo> &infos, const std::string &key)
{
    LOGD("query gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.queryBy(infos, GatewayTableInfo(key), "");
}

/*}}}*/

HBDatabase& mainDB()
{
    if (0 == gDB)
        gDB = new HBDatabase();
    return *gDB;
}

} /* namespace HB */
