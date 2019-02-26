/***************************************************************************
 *  DeviceProfileTable.cpp -
 *
 *  Created: 2018-12-17 15:29:24
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DeviceProfileTable.h"
#include "Log.h"

#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_DEVTYPE       "type"
#define FIELD_DEVNAME       "name"
#define FIELD_DEVMANU       "manufacturer"
#define FIELD_DEVVER        "version"
#define FIELD_ICONID        "iconid"
#define FIELD_JSONDATA      "json"
#define FIELD_SCRIPTDATA    "script"

DeviceProfileTable::DeviceProfileTable(SQLiteDatabase &db)
    : SQLiteTable(db, "devices")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_DEVTYPE).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_DEVNAME).append(" TEXT, ");
    sql0.append(FIELD_DEVMANU).append(" TEXT, ");
    sql0.append(FIELD_DEVVER).append(" TEXT, ");
    sql0.append(FIELD_ICONID).append(" TEXT, ");
    sql0.append(FIELD_JSONDATA).append(" TEXT, ");
    sql0.append(FIELD_SCRIPTDATA).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

DeviceProfileTable::~DeviceProfileTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool DeviceProfileTable::updateOrInsert(const DeviceTableInfo &info)
{/*{{{*/
    LOGD("update rule: %s\n", info.nDeviceType.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[7];
    values[0] = SQLText(info.nDeviceType);
    values[1] = SQLText(info.nDeviceName);
    values[2] = SQLText(info.nDeviceManu);
    values[3] = SQLText(info.nDeviceVer);
    values[4] = SQLText(info.nIconId);
    values[5] = SQLText(info.nJsonData);
    values[6] = SQLText(info.nScriptData);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_DEVTYPE).append(", ");
    sql.append(FIELD_DEVNAME).append(", ");
    sql.append(FIELD_DEVMANU).append(", ");
    sql.append(FIELD_DEVVER).append(", ");
    sql.append(FIELD_ICONID).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA).append(") VALUES(?, ?, ?, ?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool DeviceProfileTable::deleteBy(const DeviceTableInfo &info, const std::string &where)
{/*{{{*/
    LOGD("delete rule\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    /* delete by parameter where */
    if (!where.empty()) {
        sql.append(" WHERE ").append(where);
        return mDB.exec(sql.c_str());
    }

    /* delete by primary key */
    if (!info.nDeviceType.empty()) {
        sql.append(" WHERE ").append(FIELD_DEVTYPE).append(" = '").append(info.nDeviceType).append("'");
        return mDB.exec(sql.c_str());
    }

    /* delete by other key */
    bool flag = false;
    if (!info.nDeviceName.empty()) { /* filter device name */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_DEVNAME).append(" = '").append(info.nDeviceName).append("'");
        flag = true;
    }
    if (!info.nDeviceVer.empty()) { /* filter device version */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_DEVVER).append(" = '").append(info.nDeviceVer).append("'");
        flag = true;
    }
    return mDB.exec(sql.c_str());
}/*}}}*/

bool DeviceProfileTable::queryBy(std::vector<DeviceTableInfo> &infos, const DeviceTableInfo &filter, const std::string &where)
{/*{{{*/
    LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_DEVTYPE).append(", ");
    sql.append(FIELD_DEVNAME).append(", ");
    sql.append(FIELD_DEVMANU).append(", ");
    sql.append(FIELD_DEVVER).append(", ");
    sql.append(FIELD_ICONID).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nDeviceType.empty())
            sql.append(" WHERE ").append(FIELD_DEVTYPE).append(" = '").append(filter.nDeviceType).append("'");
        else {
            bool flag = false;
            if (!filter.nDeviceName.empty()) { /* filter device name */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEVNAME).append(" = '").append(filter.nDeviceName).append("'");
                flag = true;
            }
            if (!filter.nDeviceVer.empty()) { /* filter device version */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEVVER).append(" = '").append(filter.nDeviceVer).append("'");
                flag = true;
            }
        }
    } else
        sql.append(" WHERE ").append(where);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            DeviceTableInfo info;
            info.nDeviceType = assignSafe(rs->columnText(0));
            info.nDeviceName = assignSafe(rs->columnText(1));
            info.nDeviceManu = assignSafe(rs->columnText(2));
            info.nDeviceVer = assignSafe(rs->columnText(3));
            info.nIconId = assignSafe(rs->columnText(4));
            info.nJsonData = std::move(assignSafe(rs->columnText(5)));
            info.nScriptData = std::move(assignSafe(rs->columnText(6)));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
