/***************************************************************************
 *  LogTable.h - Global Table
 *
 *  Created: 2018-12-03 21:17:59
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __HBGlobalTable_H__
#define __HBGlobalTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include "Log.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct LogTableInfo {
    LogTableInfo()
        : nModule(""), nLevel(LOG_LEVEL_ERROR) {}
    std::string nModule;
    int nLevel;
}; /* struct LogTableInfo */

class HBDatabase;
class LogTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    LogTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~LogTable();

    bool updateOrInsert(const LogTableInfo &info);
    bool deleteBy(const LogTableInfo &info, const std::string &where);
    bool queryBy(std::vector<LogTableInfo> &infos, const LogTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class LogTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __HBGlobalTable_H__ */
