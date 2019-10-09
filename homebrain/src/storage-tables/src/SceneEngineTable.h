/***************************************************************************
 *  SceneEngineTable.h - Scene Engine Table
 *
 *  Created: 2019-06-05 18:14:45
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __SceneEngineTable_H__
#define __SceneEngineTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct SceneTableInfo {
    SceneTableInfo(std::string key = "")
        : nSceneId(key), nSceneName("")
        , nScriptData(""), nJsonData("")
        { }
    std::string nSceneId;
    std::string nSceneName;
    std::string nScriptData;
    std::string nJsonData;
}; /* struct SceneTableInfo */

class HBDatabase;
class SceneEngineTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    SceneEngineTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~SceneEngineTable();

    bool updateOrInsert(const SceneTableInfo &info);
    bool deleteBy(const SceneTableInfo &info, const std::string &where);
    bool queryBy(std::vector<SceneTableInfo> &infos, const SceneTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class SceneEngineTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __SceneEngineTable_H__ */
