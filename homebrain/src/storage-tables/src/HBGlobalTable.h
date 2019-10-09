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
#include <vector>

#ifdef __cplusplus

namespace HB {

struct LogTableInfo {
    LogTableInfo()
        : nModule(""), nLevel(0) {}
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
    bool queryBy(std::vector<LogTableInfo> &infos, const LogTableInfo &filter, const std::string &/*where*/);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class LogTable */


#define SCENE_MODE_FILED "scene_mode"

#define LOG_FTP_FIELD       "log_ftp"
#define LOG_FILESIZE_FIELD  "log_filesize"

struct SmartHomeInfo {
    SmartHomeInfo(): nName(""), nValue("") {}
    SmartHomeInfo(std::string name): nName(name), nValue("") {}
    SmartHomeInfo(std::string name, std::string value)
        : nName(name), nValue(value) {}
    std::string nName;
    std::string nValue;
}; /* struct SmartHomeInfo */

class SmartHomeTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    SmartHomeTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~SmartHomeTable();

    bool updateOrInsert(const SmartHomeInfo &info);
    bool deleteBy(const SmartHomeInfo &info, const std::string &where);
    bool queryBy(std::vector<SmartHomeInfo> &infos, const SmartHomeInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex;

}; /* class SmartHomeTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __HBGlobalTable_H__ */
