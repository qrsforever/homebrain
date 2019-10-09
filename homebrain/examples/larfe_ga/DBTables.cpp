/***************************************************************************
 *  DBTables.cpp -
 *
 *  Created: 2019-07-11 11:20:10
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DBTables.h"
#include "SQLiteLog.h"
#include "Common.h"

namespace HB {

using namespace UTILS;

#define FIELD_LF_NAME       "name"
#define FIELD_LF_VALUE      "value"

#define FIELD_MODULE        "module"
#define FIELD_LEVEL         "level"

#define FIELD_DEV_DID       "did"
#define FIELD_DEV_NAME      "name"
#define FIELD_DEV_KEY       "key"
#define FIELD_DEV_EID       "eid"
#define FIELD_DEV_SECRET    "secret"
#define FIELD_DEV_ROOM      "room"
#define FIELD_DEV_TOPO      "topo"
#define FIELD_DEV_TYPE      "type"

LFGlobalTable::LFGlobalTable(SQLiteDatabase &db)
    : SQLiteTable(db, "global")
{/*{{{*/
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_LF_NAME).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_LF_VALUE).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}/*}}}*/

LFGlobalTable::~LFGlobalTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool LFGlobalTable::updateOrInsert(const LFGlobalInfo &info)
{/*{{{*/
    SQL_LOGI("update: %s %s\n", info.nName.c_str(), info.nValue.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[2];
    values[0] = SQLText(info.nName);
    values[1] = SQLText(info.nValue);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_LF_NAME).append(", ");
    sql.append(FIELD_LF_VALUE).append(") VALUES(?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool LFGlobalTable::deleteBy(const LFGlobalInfo &info, const std::string &/*where*/)
{/*{{{*/
    SQL_LOGI("delete: %s\n", info.nName.c_str());

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    if (!info.nName.empty()) {
        sql.append(" WHERE ").append(FIELD_LF_NAME).append(" = '").append(info.nName).append("'");
        return mDB.exec(sql.c_str());
    }
    return false;
}/*}}}*/

bool LFGlobalTable::queryBy(std::vector<LFGlobalInfo> &infos, const LFGlobalInfo &filter, const std::string &/*where*/)
{/*{{{*/
    SQL_LOGI("query: %s\n", filter.nName.c_str());

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_LF_NAME).append(", ");
    sql.append(FIELD_LF_VALUE);
    sql.append(" FROM ").append(tableName());
    if (!filter.nName.empty())
        sql.append(" WHERE ").append(FIELD_LF_NAME).append(" = '").append(filter.nName).append("'");

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            LFGlobalInfo info;
            info.nName = assignSafe(rs->columnText(0));
            info.nValue = assignSafe(rs->columnText(1));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

void LFGlobalTable::dump()
{/*{{{*/
    std::vector<LFGlobalInfo> infos;
    if (queryBy(infos, LFGlobalInfo(), "")) {
        SQL_LOGA("dump table [%s]\n", tableName().c_str());
        for (size_t i = 0, l = infos.size(); i < l; ++i)
            SQL_LOGA("%s\n", infos[i].toString().c_str());
    }
}/*}}}*/

LFLogTable::LFLogTable(SQLiteDatabase &db)
    : SQLiteTable(db, "log")
{/*{{{*/
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_MODULE).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_LEVEL).append(" INTEGER DEFAULT 2)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}/*}}}*/

LFLogTable::~LFLogTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool LFLogTable::updateOrInsert(const LFLogInfo &info)
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

bool LFLogTable::deleteBy(const LFLogInfo &/*info*/, const std::string &/*where*/)
{/*{{{*/
    return true;
}/*}}}*/

bool LFLogTable::queryBy(std::vector<LFLogInfo> &infos, const LFLogInfo &filter, const std::string &where)
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
            LFLogInfo info;
            info.nModule = assignSafe(rs->columnText(0));
            info.nLevel = rs->columnInt(1);
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

void LFLogTable::dump()
{/*{{{*/
    std::vector<LFLogInfo> infos;
    if (queryBy(infos, LFLogInfo(), "")) {
        SQL_LOGA("dump table [%s]\n", tableName().c_str());
        for (size_t i = 0, l = infos.size(); i < l; ++i)
            SQL_LOGA("%s\n", infos[i].toString().c_str());
    }
}/*}}}*/

LFDeviceTable::LFDeviceTable(SQLiteDatabase &db)
    : SQLiteTable(db, "devices")
{/*{{{*/
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_DEV_DID).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_DEV_NAME).append(" TEXT, ");
    sql0.append(FIELD_DEV_KEY).append(" TEXT, ");
    sql0.append(FIELD_DEV_EID).append(" TEXT, ");
    sql0.append(FIELD_DEV_SECRET).append(" TEXT, ");
    sql0.append(FIELD_DEV_ROOM).append(" TEXT, ");
    sql0.append(FIELD_DEV_TOPO).append(" INTEGER DEFAULT 0, ");
    sql0.append(FIELD_DEV_TYPE).append(" INTEGER DEFAULT -1)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* v1: alert table for topo field */
    // std::string sql1("ALTER TABLE ");
    // sql1.append(tableName()).append(" ADD COLUMN ").append(FIELD_DEV_TOPO).append(" INT DEFAULT(0)");
    // mUpdateHistoryList.push_back(std::make_pair(1, sql1));

    /* check table upgrade */
    init();
}/*}}}*/

LFDeviceTable::~LFDeviceTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool LFDeviceTable::updateOrInsert(const LFDeviceInfo &info)
{/*{{{*/
    SQL_LOGI("update device: %s\n", info.nDid.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[8];
    values[0] = SQLText(info.nDid);
    values[1] = SQLText(info.nName);
    values[2] = SQLText(info.nKey);
    values[3] = SQLText(info.nEid);
    values[4] = SQLText(info.nSecret);
    values[5] = SQLText(info.nRoom);
    values[6] = SQLInt(info.nTopo);
    values[7] = SQLInt(info.nType);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_DEV_DID).append(", ");
    sql.append(FIELD_DEV_NAME).append(", ");
    sql.append(FIELD_DEV_KEY).append(", ");
    sql.append(FIELD_DEV_EID).append(", ");
    sql.append(FIELD_DEV_SECRET).append(", ");
    sql.append(FIELD_DEV_ROOM).append(", ");
    sql.append(FIELD_DEV_TOPO).append(", ");
    sql.append(FIELD_DEV_TYPE).append(") VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool LFDeviceTable::deleteBy(const LFDeviceInfo &info, const std::string &/*where*/)
{/*{{{*/
    SQL_LOGD("delete device: %s\n", info.nDid.c_str());

    if (info.nDid.empty())
        return false;

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    if (!info.nDid.empty()) {
        sql.append(" WHERE ").append(FIELD_DEV_DID).append(" = '").append(info.nDid).append("'");
        return mDB.exec(sql.c_str());
    }
    return false;
}/*}}}*/

bool LFDeviceTable::queryBy(std::vector<LFDeviceInfo> &infos, const LFDeviceInfo &filter, const std::string &where)
{/*{{{*/
    SQL_LOGI("query device: %s \n", filter.nDid.c_str());

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_DEV_DID).append(", ");
    sql.append(FIELD_DEV_NAME).append(", ");
    sql.append(FIELD_DEV_KEY).append(", ");
    sql.append(FIELD_DEV_EID).append(", ");
    sql.append(FIELD_DEV_SECRET).append(", ");
    sql.append(FIELD_DEV_ROOM).append(", ");
    sql.append(FIELD_DEV_TOPO).append(", ");
    sql.append(FIELD_DEV_TYPE);
    sql.append(" FROM ").append(tableName());

    if (where.empty()) {
        if (!filter.nDid.empty())
            sql.append(" WHERE ").append(FIELD_DEV_DID).append(" = '").append(filter.nDid).append("'");
        else {
            bool flag = false;
            if (!filter.nEid.empty()) { /* filter eid */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEV_EID).append(" = '").append(filter.nEid).append("'");
                flag = true;
            }
            if (-1 != filter.nTopo) {
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEV_TOPO).append(" = ").append(int2String(filter.nTopo));
                flag = true;
            }
            if (-1 != filter.nType) {
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DEV_TYPE).append(" = ").append(int2String(filter.nType));
                flag = true;
            }
        }
    } else
        sql.append(" WHERE ").append(where);

    SQL_LOGI("sql: %s\n", sql.c_str());
    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            LFDeviceInfo info;
            info.nDid    = assignSafe(rs->columnText(0));
            info.nName   = assignSafe(rs->columnText(1));
            info.nKey    = assignSafe(rs->columnText(2));
            info.nEid    = assignSafe(rs->columnText(3));
            info.nSecret = assignSafe(rs->columnText(4));
            info.nRoom   = assignSafe(rs->columnText(5));
            info.nTopo   = rs->columnInt(6);
            info.nType   = rs->columnInt(7);
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

void LFDeviceTable::dump()
{/*{{{*/
    std::vector<LFDeviceInfo> infos;
    if (queryBy(infos, LFDeviceInfo(), "")) {
        SQL_LOGA("dump table [%s]\n", tableName().c_str());
        for (size_t i = 0, l = infos.size(); i < l; ++i)
            SQL_LOGA("%s\n", infos[i].toString().c_str());
    }
}/*}}}*/


} /* namespace HB */
