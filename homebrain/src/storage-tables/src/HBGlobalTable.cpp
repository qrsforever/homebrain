/***************************************************************************
 *  LogTable.cpp - Global Table
 *
 *  Created: 2018-12-03 21:17:59
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBGlobalTable.h"
#include "SQLiteLog.h"
#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_MODULE        "module"
#define FIELD_LEVEL         "level"

#define FIELD_SH_NAME       "name"
#define FIELD_SH_VALUE      "value"

LogTable::LogTable(SQLiteDatabase &db)
    : SQLiteTable(db, "log")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_MODULE).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_LEVEL).append(" INTEGER DEFAULT 2)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

LogTable::~LogTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool LogTable::updateOrInsert(const LogTableInfo &info)
{/*{{{*/
    SQL_LOGD("update log: %s\n", info.nModule.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[2];
    values[0] = SQLText(info.nModule);
    values[1] = SQLInt(info.nLevel);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_MODULE).append(", ");
    sql.append(FIELD_LEVEL).append(") VALUES(?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool LogTable::deleteBy(const LogTableInfo &/*info*/, const std::string &/*where*/)
{/*{{{*/
    return true;
}/*}}}*/

bool LogTable::queryBy(std::vector<LogTableInfo> &infos, const LogTableInfo &filter, const std::string &where)
{/*{{{*/
    SQL_LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_MODULE).append(", ");
    sql.append(FIELD_LEVEL);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nModule.empty())
            sql.append(" WHERE ").append(FIELD_MODULE).append(" = '").append(filter.nModule).append("'");
    } else
        sql.append(" WHERE ").append(where);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            LogTableInfo info;
            info.nModule = assignSafe(rs->columnText(0));
            info.nLevel = rs->columnInt(1);
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/


SmartHomeTable::SmartHomeTable(SQLiteDatabase &db)
    : SQLiteTable(db, "smarthome")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_SH_NAME).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_SH_VALUE).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

SmartHomeTable::~SmartHomeTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool SmartHomeTable::updateOrInsert(const SmartHomeInfo &info)
{/*{{{*/
    SQL_LOGI("update sh: %s %s\n", info.nName.c_str(), info.nValue.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[2];
    values[0] = SQLText(info.nName);
    values[1] = SQLText(info.nValue);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_SH_NAME).append(", ");
    sql.append(FIELD_SH_VALUE).append(") VALUES(?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool SmartHomeTable::deleteBy(const SmartHomeInfo &info, const std::string &/*where*/)
{/*{{{*/
    SQL_LOGD("delete rule\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    if (!info.nName.empty()) {
        sql.append(" WHERE ").append(FIELD_SH_NAME).append(" = '").append(info.nName).append("'");
        return mDB.exec(sql.c_str());
    }
    return false;
}/*}}}*/

bool SmartHomeTable::queryBy(std::vector<SmartHomeInfo> &infos, const SmartHomeInfo &filter, const std::string &/*where*/)
{/*{{{*/
    SQL_LOGD("query sh\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_SH_NAME).append(", ");
    sql.append(FIELD_SH_VALUE);
    sql.append(" FROM ").append(tableName());
    if (!filter.nName.empty())
        sql.append(" WHERE ").append(FIELD_SH_NAME).append(" = '").append(filter.nName).append("'");

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            SmartHomeInfo info;
            info.nName = assignSafe(rs->columnText(0));
            info.nValue = assignSafe(rs->columnText(1));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
