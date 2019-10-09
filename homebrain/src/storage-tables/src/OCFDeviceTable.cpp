/***************************************************************************
 *  OCFDeviceTable.cpp - OCF device Table
 *
 *  Created: 2018-11-12 16:20:53
 *
 *  Copyright QRS
 ****************************************************************************/

#include "OCFDeviceTable.h"
#include "SQLiteLog.h"
#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_DEVICEID        "id"
#define FIELD_DEVICENAME      "name"
#define FIELD_DEVICETYPE      "type"
#define FIELD_MANUFACTURE     "manufacture"
#define FIELD_ALIAS           "alias"
#define FIELD_ROOM            "room"

OCFDeviceTable::OCFDeviceTable(SQLiteDatabase &db)
    : SQLiteTable(db, "devices")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_DEVICEID).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_DEVICENAME).append(" TEXT, ");
    sql0.append(FIELD_DEVICETYPE).append(" TEXT, ");
    sql0.append(FIELD_MANUFACTURE).append(" TEXT, ");
    sql0.append(FIELD_ALIAS).append(" TEXT, ");
    sql0.append(FIELD_ROOM).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

OCFDeviceTable::~OCFDeviceTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool OCFDeviceTable::updateOrInsert(const OCFDeviceTableInfo &info)
{/*{{{*/
    SQL_LOGD("update device: %s: %s\n", info.nDeviceId.c_str(), info.nAlias.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[6];
    values[0] = SQLText(info.nDeviceId);
    values[1] = SQLText(info.nDeviceName);
    values[2] = SQLText(info.nDeviceType);
    values[3] = SQLText(info.nManufacture);
    values[4] = SQLText(info.nAlias);
    values[5] = SQLText(info.nRoom);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_DEVICEID).append(", ");
    sql.append(FIELD_DEVICENAME).append(", ");
    sql.append(FIELD_DEVICETYPE).append(", ");
    sql.append(FIELD_MANUFACTURE).append(", ");
    sql.append(FIELD_ALIAS).append(", ");
    sql.append(FIELD_ROOM);
    sql.append(") VALUES(?, ?, ?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool OCFDeviceTable::deleteBy(const OCFDeviceTableInfo &info, const std::string &where)
{/*{{{*/
    SQL_LOGD("delete device\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    /* delete by parameter where */
    if (!where.empty()) {
        sql.append(" WHERE ").append(where);
        return mDB.exec(sql.c_str());
    }

    /* delete by primary key */
    if (!info.nDeviceId.empty()) {
        sql.append(" WHERE ").append(FIELD_DEVICEID).append(" = '").append(info.nDeviceId).append("'");
        return mDB.exec(sql.c_str());
    }

    /* delete by other key */
    bool flag = false;
    if (!info.nDeviceName.empty()) { /* filter device name */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_DEVICENAME).append(" = '").append(info.nDeviceName).append("'");
        flag = true;
    }
    if (!info.nDeviceType.empty()) { /* filter device type */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_DEVICETYPE).append(" = '").append(info.nDeviceType).append("'");
        flag = true;
    }
    if (info.nManufacture.empty()) { /* filter manufacture */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_MANUFACTURE).append(" = '").append(info.nManufacture).append("'");
        flag = true;
    }
    return mDB.exec(sql.c_str());
}/*}}}*/

bool OCFDeviceTable::queryBy(std::vector<OCFDeviceTableInfo> &infos, const OCFDeviceTableInfo &filter, const std::string &where)
{/*{{{*/
    SQL_LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_DEVICEID).append(", ");
    sql.append(FIELD_DEVICENAME).append(", ");
    sql.append(FIELD_DEVICETYPE).append(", ");
    sql.append(FIELD_MANUFACTURE).append(", ");
    sql.append(FIELD_ALIAS).append(", ");
    sql.append(FIELD_ROOM);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nDeviceId.empty())
            sql.append(" WHERE ").append(FIELD_DEVICEID).append(" = '").append(filter.nDeviceId).append("'");
        else {
            bool flag = false;
            if (!filter.nDeviceName.empty()) { /* filter device name */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEVICENAME).append(" = '").append(filter.nDeviceName).append("'");
                flag = true;
            }
            if (!filter.nDeviceType.empty()) { /* filter device type */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEVICETYPE).append(" = '").append(filter.nDeviceType).append("'");
                flag = true;
            }
            if (!filter.nManufacture.empty()) { /* filter manufacture */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_MANUFACTURE).append(" = '").append(filter.nManufacture).append("'");
                flag = true;
            }
        }
    } else
        sql.append(" WHERE ").append(where);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            OCFDeviceTableInfo info;
            info.nDeviceId = assignSafe(rs->columnText(0));
            info.nDeviceName = assignSafe(rs->columnText(1));
            info.nDeviceType = assignSafe(rs->columnText(2));
            info.nManufacture= assignSafe(rs->columnText(3));
            info.nAlias = assignSafe(rs->columnText(4));
            info.nRoom = assignSafe(rs->columnText(5));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
