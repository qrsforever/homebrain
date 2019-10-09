/***************************************************************************
 *  GatewayConnectTable.cpp -
 *
 *  Created: 2019-02-20 16:32:17
 *
 *  Copyright QRS
 ****************************************************************************/

#include "GatewayConnectTable.h"
#include "SQLiteLog.h"
#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_GID      "id"
#define FIELD_DID      "did"
#define FIELD_TYPE     "type"
#define FIELD_KEY      "key"
#define FIELD_IP       "ip"
#define FIELD_ENABLE   "enable"

GatewayConnectTable::GatewayConnectTable(SQLiteDatabase &db)
    : SQLiteTable(db, "gateway")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_GID).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_DID).append(" TEXT, ");
    sql0.append(FIELD_TYPE).append(" TEXT, ");
    sql0.append(FIELD_KEY).append(" TEXT, ");
    sql0.append(FIELD_IP).append(" TEXT, ");
    sql0.append(FIELD_ENABLE).append(" INTEGER DEFAULT 0)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

GatewayConnectTable::~GatewayConnectTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool GatewayConnectTable::updateOrInsert(const GatewayTableInfo &info)
{/*{{{*/
    SQL_LOGD("update gateway: %s\n", info.nGid.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[6];
    values[0] = SQLText(info.nGid);
    values[1] = SQLText(info.nDid);
    values[2] = SQLText(info.nType);
    values[3] = SQLText(info.nKey);
    values[4] = SQLText(info.nIp);
    values[5] = SQLInt(info.nEnable);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_GID).append(", ");
    sql.append(FIELD_DID).append(", ");
    sql.append(FIELD_TYPE).append(", ");
    sql.append(FIELD_KEY).append(", ");
    sql.append(FIELD_IP).append(", ");
    sql.append(FIELD_ENABLE).append(") VALUES(?, ?, ?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool GatewayConnectTable::deleteBy(const GatewayTableInfo &info, const std::string &where)
{/*{{{*/
    SQL_LOGD("delete gateway\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    /* delete by parameter where */
    if (!where.empty()) {
        sql.append(" WHERE ").append(where);
        return mDB.exec(sql.c_str());
    }

    /* delete by primary key */
    if (!info.nGid.empty()) {
        sql.append(" WHERE ").append(FIELD_GID).append(" = '").append(info.nGid).append("'");
        return mDB.exec(sql.c_str());
    }

    /* delete by other key */
    bool flag = false;
    if (!info.nDid.empty()) {
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_DID).append(" = '").append(info.nDid).append("'");
        flag = true;
    }
    if (!info.nKey.empty()) {
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_KEY).append(" = '").append(info.nKey).append("'");
        flag = true;
    }
    if (-1 != info.nEnable) {
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_ENABLE).append(" = ").append(int2String(info.nEnable));
        flag = true;
    }
    return mDB.exec(sql.c_str());
}/*}}}*/

bool GatewayConnectTable::queryBy(std::vector<GatewayTableInfo> &infos, const GatewayTableInfo &filter, const std::string &where)
{/*{{{*/
    SQL_LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_GID).append(", ");
    sql.append(FIELD_DID).append(", ");
    sql.append(FIELD_TYPE).append(", ");
    sql.append(FIELD_KEY).append(", ");
    sql.append(FIELD_IP).append(", ");
    sql.append(FIELD_ENABLE);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nGid.empty())
            sql.append(" WHERE ").append(FIELD_GID).append(" = '").append(filter.nGid).append("'");
        else {
            bool flag = false;
            if (!filter.nDid.empty()) {
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_DID).append(" = '").append(filter.nDid).append("'");
                flag = true;
            }
            if (!filter.nKey.empty()) {
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_KEY).append(" = '").append(filter.nKey).append("'");
                flag = true;
            }
            if (-1 != filter.nEnable) {
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_ENABLE).append(" = ").append(int2String(filter.nEnable));
                flag = true;
            }
        }
    } else
        sql.append(" WHERE ").append(where);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            GatewayTableInfo info;
            info.nGid = assignSafe(rs->columnText(0));
            info.nDid = assignSafe(rs->columnText(1));
            info.nType = assignSafe(rs->columnText(2));
            info.nKey = assignSafe(rs->columnText(3));
            info.nIp = assignSafe(rs->columnText(4));
            info.nEnable = rs->columnInt(5);
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
