/***************************************************************************
 *  DBTables.h -
 *
 *  Created: 2019-09-27
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

#define KKGA_KEY_FIELD      "kkga_key"
#define CUR_VERSION_FIELD   "cur_version"
#define LOG_FTP_FIELD       "log_ftp"
#define LOG_FILESIZE_FIELD  "log_filesize"

struct KKGlobalInfo {
    KKGlobalInfo(): nName(""), nValue("") {}
    KKGlobalInfo(std::string name): nName(name), nValue("") {}
    KKGlobalInfo(std::string name, std::string value)
        : nName(name), nValue(value) {}
    std::string nName;
    std::string nValue;
    std::string toString() {
        char info[128] = { 0 };
        snprintf(info, 127, "Name[%s] Value[%s]", nName.c_str(), nValue.c_str());
        return info;
    }
}; /* struct KKGlobalInfo */

class KKGlobalTable : public ::UTILS::SQLiteTable {
private:
    friend class KKDatabase;
    KKGlobalTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~KKGlobalTable();

    int tableType() { return TABLE_TYPE_GLOBAL; }
    bool updateOrInsert(const KKGlobalInfo &info);
    bool deleteBy(const KKGlobalInfo &info, const std::string &where);
    bool queryBy(std::vector<KKGlobalInfo> &infos, const KKGlobalInfo &filter, const std::string &where);
    void dump();
private:
    ::UTILS::Mutex mMutex;

}; /* class KKGlobalTable */

/*-----------------------------------------------------------------
 *  Log level table
 *-----------------------------------------------------------------*/

struct KKLogInfo {
    KKLogInfo()
        : nModule(""), nLevel(0) {}
    std::string nModule;
    int nLevel;
    std::string toString() {
        char info[128] = { 0 };
        snprintf(info, 127, "Module[%s] Level[%d]", nModule.c_str(), nLevel);
        return info;
    }
}; /* struct KKLogInfo */

class KKLogTable : public ::UTILS::SQLiteTable {
private:
    friend class KKDatabase;
    KKLogTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~KKLogTable();

    bool updateOrInsert(const KKLogInfo &info);
    bool deleteBy(const KKLogInfo &info, const std::string &where);
    bool queryBy(std::vector<KKLogInfo> &infos, const KKLogInfo &filter, const std::string &/*where*/);
    void dump();
private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class KKLogTable */


/*-----------------------------------------------------------------
 *  Device table
 *-----------------------------------------------------------------*/

struct KKDeviceInfo {
    KKDeviceInfo()
        : nDid(""), nName(""), nKey("")
        , nEid(""), nSecret(""), nRoom(""), nTopo(-1), nType(-1) {}
    KKDeviceInfo(std::string did)
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

class KKDeviceTable : public ::UTILS::SQLiteTable {
private:
    friend class KKDatabase;
    KKDeviceTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~KKDeviceTable();

    int tableType() { return TABLE_TYPE_DEVICE; }
    bool updateOrInsert(const KKDeviceInfo &info);
    bool deleteBy(const KKDeviceInfo &info, const std::string &where);
    bool queryBy(std::vector<KKDeviceInfo> &infos, const KKDeviceInfo &filter, const std::string &where);
    void dump();
private:
    ::UTILS::Mutex mMutex;

}; /* class KKDeviceTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __DBTables_H__ */
