/***************************************************************************
 *  SceneEngineTable.cpp -
 *
 *  Created: 2019-06-05 18:17:48
 *
 *  Copyright QRS
 ****************************************************************************/

#include "SceneEngineTable.h"
#include "SQLiteLog.h"
#include "Common.h"

using namespace UTILS;

namespace HB {

#define FIELD_RULEID        "id"
#define FIELD_RULENAME      "name"
#define FIELD_JSONDATA      "json"
#define FIELD_SCRIPTDATA    "script"

SceneEngineTable::SceneEngineTable(SQLiteDatabase &db)
    : SQLiteTable(db, "scenes")
{
    /* v0: create base table */
    std::string sql0("CREATE TABLE ");
    sql0.append(tableName()).append("(");
    sql0.append(FIELD_RULEID).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql0.append(FIELD_RULENAME).append(" TEXT, ");
    sql0.append(FIELD_JSONDATA).append(" TEXT, ");
    sql0.append(FIELD_SCRIPTDATA).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql0));

    /* check table upgrade */
    init();
}

SceneEngineTable::~SceneEngineTable()
{
    Mutex::Autolock _l(&mMutex);
}

bool SceneEngineTable::updateOrInsert(const SceneTableInfo &info)
{/*{{{*/
    SQL_LOGD("update rule: %s\n", info.nSceneId.c_str());

    Mutex::Autolock _l(&mMutex);

    SQLiteValue values[4];
    values[0] = SQLText(info.nSceneId);
    values[1] = SQLText(info.nSceneName);
    values[2] = SQLText(info.nJsonData);
    values[3] = SQLText(info.nScriptData);

    std::string sql("REPLACE INTO ");
    sql.append(tableName()).append("(");
    sql.append(FIELD_RULEID).append(", ");
    sql.append(FIELD_RULENAME).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA).append(") VALUES(?, ?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}/*}}}*/

bool SceneEngineTable::deleteBy(const SceneTableInfo &info, const std::string &where)
{/*{{{*/
    SQL_LOGD("delete rule\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("DELETE FROM ");
    sql.append(tableName());

    /* delete by parameter where */
    if (!where.empty()) {
        sql.append(" WHERE ").append(where);
        return mDB.exec(sql.c_str());
    }

    /* delete by primary key */
    if (!info.nSceneId.empty()) {
        sql.append(" WHERE ").append(FIELD_RULEID).append(" = '").append(info.nSceneId).append("'");
        return mDB.exec(sql.c_str());
    }
    return false;
}/*}}}*/

bool SceneEngineTable::queryBy(std::vector<SceneTableInfo> &infos, const SceneTableInfo &filter, const std::string &where)
{/*{{{*/
    SQL_LOGD("query\n");

    Mutex::Autolock _l(&mMutex);

    std::string sql("SELECT ");
    sql.append(FIELD_RULEID).append(", ");
    sql.append(FIELD_RULENAME).append(", ");
    sql.append(FIELD_JSONDATA).append(", ");
    sql.append(FIELD_SCRIPTDATA);
    sql.append(" FROM ").append(tableName());
    if (where.empty()) {
        if (!filter.nSceneId.empty())
            sql.append(" WHERE ").append(FIELD_RULEID).append(" = '").append(filter.nSceneId).append("'");
    } else
        sql.append(" WHERE ").append(where);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            SceneTableInfo info;
            info.nSceneId = assignSafe(rs->columnText(0));
            info.nSceneName = assignSafe(rs->columnText(1));
            info.nJsonData = std::move(assignSafe(rs->columnText(2)));
            info.nScriptData = std::move(assignSafe(rs->columnText(3)));
            infos.push_back(info);
        }
        rs->close();
    }
    return true;
}/*}}}*/

} /* namespace HB */
