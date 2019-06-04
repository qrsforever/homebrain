/***************************************************************************
 *  RuleEngineTable.cpp - Rule Engine Table
 *
 *  Created: 2018-11-12 16:20:53
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineTable.h"
#include "Log.h"

#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_RULEID        "id"
#define FIELD_RULENAME      "name"
#define FIELD_RULEDESR      "descr"
#define FIELD_RULEVER       "version"
#define FIELD_ENABLE        "enable"
#define FIELD_MANUAL        "manual"
#define FIELD_FRIENDS       "friends"
#define FIELD_JSONDATA      "json"
#define FIELD_SCRIPTDATA    "script"

RuleEngineTable::RuleEngineTable(SQLiteDatabase &db)
    : SQLiteTable(db, "rules")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_RULEID).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_RULENAME).append(" TEXT, ");
    sql0.append(FIELD_RULEDESR).append(" TEXT, ");
    sql0.append(FIELD_RULEVER).append(" TEXT, ");
    sql0.append(FIELD_ENABLE).append(" INTEGER DEFAULT 0, ");
    sql0.append(FIELD_MANUAL).append(" INTEGER DEFAULT 0, ");
    sql0.append(FIELD_FRIENDS).append(" TEXT, ");
    sql0.append(FIELD_JSONDATA).append(" TEXT, ");
    sql0.append(FIELD_SCRIPTDATA).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

RuleEngineTable::~RuleEngineTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool RuleEngineTable::updateOrInsert(const RuleTableInfo &info)
{/*{{{*/
    LOGD("update rule: %s\n", info.nRuleId.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[9];
    values[0] = SQLText(info.nRuleId);
    values[1] = SQLText(info.nRuleName);
    values[2] = SQLText(info.nRuleDesr);
    values[3] = SQLText(info.nRuleVer);
    values[4] = SQLInt(info.nEnable);
    values[5] = SQLInt(info.nManual);
    values[6] = SQLText(info.nFriends);
    values[7] = SQLText(info.nJsonData);
    values[8] = SQLText(info.nScriptData);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_RULEID).append(", ");
    sql.append(FIELD_RULENAME).append(", ");
    sql.append(FIELD_RULEDESR).append(", ");
    sql.append(FIELD_RULEVER).append(", ");
    sql.append(FIELD_ENABLE).append(", ");
    sql.append(FIELD_MANUAL).append(", ");
    sql.append(FIELD_FRIENDS).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA).append(") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool RuleEngineTable::deleteBy(const RuleTableInfo &info, const std::string &where)
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
    if (!info.nRuleId.empty()) {
        sql.append(" WHERE ").append(FIELD_RULEID).append(" = '").append(info.nRuleId).append("'");
        return mDB.exec(sql.c_str());
    }

    /* delete by other key */
    bool flag = false;
    if (!info.nRuleName.empty()) { /* filter rule name */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_RULENAME).append(" = '").append(info.nRuleName).append("'");
        flag = true;
    }
    if (!info.nRuleVer.empty()) { /* filter rule version */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_RULEVER).append(" = '").append(info.nRuleVer).append("'");
        flag = true;
    }
    if (-1 != info.nEnable) { /* filter rule enable */
        sql.append(flag ? " AND " : " WHERE ");
        sql.append(FIELD_ENABLE).append(" = ").append(int2String(info.nEnable));
        flag = true;
    }
    return mDB.exec(sql.c_str());
}/*}}}*/

bool RuleEngineTable::queryBy(std::vector<RuleTableInfo> &infos, const RuleTableInfo &filter, const std::string &where)
{/*{{{*/
    LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_RULEID).append(", ");
    sql.append(FIELD_RULENAME).append(", ");
    sql.append(FIELD_RULEDESR).append(", ");
    sql.append(FIELD_RULEVER).append(", ");
    sql.append(FIELD_ENABLE).append(", ");
    sql.append(FIELD_MANUAL).append(", ");
    sql.append(FIELD_FRIENDS).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nRuleId.empty())
            sql.append(" WHERE ").append(FIELD_RULEID).append(" = '").append(filter.nRuleId).append("'");
        else {
            bool flag = false;
            if (!filter.nRuleName.empty()) { /* filter rule name */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_RULENAME).append(" = '").append(filter.nRuleName).append("'");
                flag = true;
            }
            if (!filter.nRuleVer.empty()) { /* filter rule version */
                sql.append(flag ? " AND " : " WHERE ");
                sql.append(FIELD_RULEVER).append(" = '").append(filter.nRuleVer).append("'");
                flag = true;
            }
            if (-1 != filter.nEnable) { /* filter rule enable */
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
            RuleTableInfo info;
            info.nRuleId = assignSafe(rs->columnText(0));
            info.nRuleName = assignSafe(rs->columnText(1));
            info.nRuleDesr = assignSafe(rs->columnText(2));
            info.nRuleVer = assignSafe(rs->columnText(3));
            info.nEnable = rs->columnInt(4);
            info.nManual = rs->columnInt(5);
            info.nFriends = std::move(assignSafe(rs->columnText(6)));
            info.nJsonData = std::move(assignSafe(rs->columnText(7)));
            info.nScriptData = std::move(assignSafe(rs->columnText(8)));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
