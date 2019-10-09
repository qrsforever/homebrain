/***************************************************************************
 *  HBDatabase.cpp - HomeBrain Database
 *
 *  Created: 2018-11-12 14:11:55
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBDatabase.h"
#include "SQLiteLog.h"
#include "HBGlobalTable.h"
#include "MessageTypes.h"
#include "MainPublicHandler.h"

#include "RuleEngineTable.h"
#include "SceneEngineTable.h"
#include "DeviceProfileTable.h"
#include "GatewayConnectTable.h"
#include "OCFDeviceTable.h"

using namespace UTILS;

namespace HB {

static HBDatabase *gDB = 0;

HBDatabase::HBDatabase() : mDBPath("engine.db"), mAutoCloseInterval(15000), mDB(0)
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

    SQL_LOGD("database[%s]\n", mDBPath.c_str());

    mDB = new SQLiteDatabase();
    if (!mDB->open(mDBPath.c_str()))
        return false;
    return true;
}

bool HBDatabase::close()
{
    SQL_LOGD("database[%s]\n", mDBPath.c_str());

    if (mDB)
        delete mDB;
    mDB = 0;

    return true;
}

/*{{{ Log Table */
template<>
bool HBDatabase::updateOrInsert<LogTableInfo>(const LogTableInfo &info)
{
    SQL_LOGI("update log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<LogTableInfo>(const LogTableInfo &info)
{
    SQL_LOGI("delete log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<LogTableInfo>(std::vector<LogTableInfo> &infos, const LogTableInfo &info)
{
    SQL_LOGI("query log table\n");

    if (!open())
        return false;

    LogTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

/*}}}*/

/*{{{ SmartHome Table */
template<>
bool HBDatabase::updateOrInsert<SmartHomeInfo>(const SmartHomeInfo &info)
{
    SQL_LOGI("update smarthome table\n");

    if (!open())
        return false;

    SmartHomeTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<SmartHomeInfo>(const SmartHomeInfo &info)
{
    SQL_LOGI("delete smarthome table\n");

    if (!open())
        return false;

    SmartHomeTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<SmartHomeInfo>(std::vector<SmartHomeInfo> &infos, const SmartHomeInfo &info)
{
    SQL_LOGI("query smarthome table\n");

    if (!open())
        return false;

    SmartHomeTable tab(*mDB);

    return tab.queryBy(infos, info, "");
}

template <>
bool HBDatabase::queryBy<SmartHomeInfo>(std::vector<SmartHomeInfo> &infos, const std::string &key)
{
    SQL_LOGD("query smarthome table\n");

    if (!open())
        return false;

    SmartHomeTable tab(*mDB);

    return tab.queryBy(infos, SmartHomeInfo(key), "");
}

/*}}}*/

/*{{{ Device Profile Table */
template<>
bool HBDatabase::updateOrInsert<DeviceTableInfo>(const DeviceTableInfo &info)
{
    SQL_LOGD("update device table\n");

    if (!open())
        return false;

    DeviceProfileTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<DeviceTableInfo>(const DeviceTableInfo &info)
{
    SQL_LOGD("delete device table\n");

    if (!open())
        return false;

    DeviceProfileTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<DeviceTableInfo>(std::vector<DeviceTableInfo> &infos, const DeviceTableInfo &info)
{
    SQL_LOGD("query device table\n");

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
    SQL_LOGD("update rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<RuleTableInfo>(const RuleTableInfo &info)
{
    SQL_LOGD("delete rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<RuleTableInfo>(std::vector<RuleTableInfo> &infos, const RuleTableInfo &filter)
{
    SQL_LOGD("query rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<RuleTableInfo>(std::vector<RuleTableInfo> &infos, const std::string &key)
{
    SQL_LOGD("query rule table\n");

    if (!open())
        return false;

    RuleEngineTable tab(*mDB);

    return tab.queryBy(infos, RuleTableInfo(key), "");
}

/*}}}*/

/*{{{ Scene Table */
template<>
bool HBDatabase::updateOrInsert<SceneTableInfo>(const SceneTableInfo &info)
{
    SQL_LOGD("update scene table\n");

    if (!open())
        return false;

    SceneEngineTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<SceneTableInfo>(const SceneTableInfo &info)
{
    SQL_LOGD("delete scene table\n");

    if (!open())
        return false;

    SceneEngineTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<SceneTableInfo>(std::vector<SceneTableInfo> &infos, const SceneTableInfo &filter)
{
    SQL_LOGD("query scene table\n");

    if (!open())
        return false;

    SceneEngineTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<SceneTableInfo>(std::vector<SceneTableInfo> &infos, const std::string &key)
{
    SQL_LOGD("query rule table\n");

    if (!open())
        return false;

    SceneEngineTable tab(*mDB);

    return tab.queryBy(infos, SceneTableInfo(key), "");
}

/*}}}*/

/*{{{ Gateway Table */
template<>
bool HBDatabase::updateOrInsert<GatewayTableInfo>(const GatewayTableInfo &info)
{
    SQL_LOGD("update gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<GatewayTableInfo>(const GatewayTableInfo &info)
{
    SQL_LOGD("delete gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<GatewayTableInfo>(std::vector<GatewayTableInfo> &infos, const GatewayTableInfo &filter)
{
    SQL_LOGD("query gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<GatewayTableInfo>(std::vector<GatewayTableInfo> &infos, const std::string &key)
{
    SQL_LOGD("query gateway table\n");

    if (!open())
        return false;

    GatewayConnectTable tab(*mDB);

    return tab.queryBy(infos, GatewayTableInfo(key), "");
}

/*}}}*/

/*{{{ Device list Table */
template<>
bool HBDatabase::updateOrInsert<OCFDeviceTableInfo>(const OCFDeviceTableInfo &info)
{
    SQL_LOGD("update device list table\n");

    if (!open())
        return false;

    OCFDeviceTable tab(*mDB);

    return tab.updateOrInsert(info);
}

template<>
bool HBDatabase::deleteBy<OCFDeviceTableInfo>(const OCFDeviceTableInfo &info)
{
    SQL_LOGD("delete item from device list table\n");

    if (!open())
        return false;

    OCFDeviceTable tab(*mDB);

    return tab.deleteBy(info, "");
}

template <>
bool HBDatabase::queryBy<OCFDeviceTableInfo>(std::vector<OCFDeviceTableInfo> &infos, const OCFDeviceTableInfo &filter)
{
    SQL_LOGD("query item from device list table\n");

    if (!open())
        return false;

    OCFDeviceTable tab(*mDB);

    return tab.queryBy(infos, filter, "");
}

template <>
bool HBDatabase::queryBy<OCFDeviceTableInfo>(std::vector<OCFDeviceTableInfo> &infos, const std::string &key)
{
    SQL_LOGD("query item from device list table\n");

    if (!open())
        return false;

    OCFDeviceTable tab(*mDB);

    return tab.queryBy(infos, OCFDeviceTableInfo(key), "");
}

/*}}}*/

HBDatabase& mainDB()
{
    if (0 == gDB)
        gDB = new HBDatabase();
    return *gDB;
}

} /* namespace HB */
