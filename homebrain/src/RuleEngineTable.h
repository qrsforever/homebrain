/***************************************************************************
 *  RuleEngineTable.h - Rule Engine Table
 *
 *  Created: 2018-11-12 16:19:08
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineTable_H__
#define __RuleEngineTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct RuleTableInfo {
    RuleTableInfo(std::string key = "")
        : nRuleId(key), nRuleVer(""), nRuleDesr("")
        , nFriends(""), nScriptData(""), nJsonData("")
        , nEnable(-1), nManual(-1) { }
    std::string nRuleId;
    std::string nRuleName;
    std::string nRuleVer;
    std::string nRuleDesr;
    std::string nFriends;
    std::string nScriptData;
    std::string nJsonData;
    int nEnable;
    int nManual;
}; /* struct RuleTableInfo */

class HBDatabase;
class RuleEngineTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    RuleEngineTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~RuleEngineTable();

    bool updateOrInsert(const RuleTableInfo &info);
    bool deleteBy(const RuleTableInfo &info, const std::string &where);
    bool queryBy(std::vector<RuleTableInfo> &infos, const RuleTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class RuleEngineTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineTable_H__ */
