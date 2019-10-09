/***************************************************************************
 *  DBTables.h -
 *
 *  Created: 2019-07-11 11:20:01
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __DBTables_H__
#define __DBTables_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

#define TABLE_TYPE_GLOBAL   1
#define TABLE_TYPE_LOGLEVEL 2
#define TABLE_TYPE_DEVICE   3

/*-----------------------------------------------------------------
 *  Global n-v table
 *-----------------------------------------------------------------*/

#define LFGA_KEY_FIELD      "lfga_key"
#define CUR_VERSION_FIELD   "cur_version"
#define LOG_FTP_FIELD       "log_ftp"
#define LOG_FILESIZE_FIELD  "log_filesize"

struct LFGlobalInfo {
    LFGlobalInfo(): nName(""), nValue("") {}
    LFGlobalInfo(std::string name): nName(name), nValue("") {}
    LFGlobalInfo(std::string name, std::string value)
        : nName(name), nValue(value) {}
    std::string nName;
    std::string nValue;
    std::string toString() {
        char info[128] = { 0 };
        snprintf(info, 127, "Name[%s] Value[%s]", nName.c_str(), nValue.c_str());
        return info;
    }
}; /* struct LFGlobalInfo */

class LFGlobalTable : public ::UTILS::SQLiteTable {
private:
    friend class LFDatabase;
    LFGlobalTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~LFGlobalTable();

    int tableType() { return TABLE_TYPE_GLOBAL; }
    bool updateOrInsert(const LFGlobalInfo &info);
    bool deleteBy(const LFGlobalInfo &info, const std::string &where);
    bool queryBy(std::vector<LFGlobalInfo> &infos, const LFGlobalInfo &filter, const std::string &where);
    void dump();
private:
    ::UTILS::Mutex mMutex;

}; /* class LFGlobalTable */

/*-----------------------------------------------------------------
 *  Log level table
 *-----------------------------------------------------------------*/

struct LFLogInfo {
    LFLogInfo()
        : nModule(""), nLevel(0) {}
    std::string nModule;
    int nLevel;
    std::string toString() {
        char info[128] = { 0 };
        snprintf(info, 127, "Module[%s] Level[%d]", nModule.c_str(), nLevel);
        return info;
    }
}; /* struct LFLogInfo */

class LFLogTable : public ::UTILS::SQLiteTable {
private:
    friend class LFDatabase;
    LFLogTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~LFLogTable();

    bool updateOrInsert(const LFLogInfo &info);
    bool deleteBy(const LFLogInfo &info, const std::string &where);
    bool queryBy(std::vector<LFLogInfo> &infos, const LFLogInfo &filter, const std::string &/*where*/);
    void dump();
private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class LFLogTable */


/*-----------------------------------------------------------------
 *  Device table
 *-----------------------------------------------------------------*/

struct LFDeviceInfo {
    LFDeviceInfo()
        : nDid(""), nName(""), nKey("")
        , nEid(""), nSecret(""), nRoom(""), nTopo(-1), nType(-1) {}
    LFDeviceInfo(std::string did)
        : nDid(did), nName(""), nKey("")
        , nEid(""), nSecret(""), nRoom(""), nTopo(-1), nType(-1) {}
    std::string nDid;
    std::string nName;
    std::string nKey;
    std::string nEid;
    std::string nSecret;
    std::string nRoom;
    int nTopo;
    int nType;
    std::string toString() {
        char info[256] = { 0 };
        snprintf(info, 255, "Did[%s] Name[%s] Key[%s] Eid[%s] Secrent[%s] Room[%s] Topo[%d] Type[%d]",
            nDid.c_str(), nName.c_str(), nKey.c_str(), nEid.c_str(), nSecret.c_str(), nRoom.c_str(),
            nTopo, nType);
        return info;
    }
};

class LFDeviceTable : public ::UTILS::SQLiteTable {
private:
    friend class LFDatabase;
    LFDeviceTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~LFDeviceTable();

    int tableType() { return TABLE_TYPE_DEVICE; }
    bool updateOrInsert(const LFDeviceInfo &info);
    bool deleteBy(const LFDeviceInfo &info, const std::string &where);
    bool queryBy(std::vector<LFDeviceInfo> &infos, const LFDeviceInfo &filter, const std::string &where);
    void dump();
private:
    ::UTILS::Mutex mMutex;

}; /* class LFDeviceTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __DBTables_H__ */
