/***************************************************************************
 *  LogTable.cpp - Global Table
 *
 *  Created: 2018-12-03 21:17:59
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBGlobalTable.h"
#include "Log.h"

#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_MODULE        "module"
#define FIELD_LEVEL         "level"

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
    LOGD("update rule: %s\n", info.nModule.c_str());

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
    LOGD("query\n");

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

} /* namespace HB */
