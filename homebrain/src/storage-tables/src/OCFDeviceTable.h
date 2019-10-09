/***************************************************************************
 *  OCFDeviceTable.h - OCF Device Table
 *
 *  Created: 2018-11-12 16:19:08
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __OCFDeviceTable_H__
#define __OCFDeviceTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct OCFDeviceTableInfo {
    OCFDeviceTableInfo(std::string key = "")
        : nDeviceId(key), nDeviceType(""),
          nDeviceName(""), nManufacture(""), nAlias("null"), nRoom("default") { }
    std::string nDeviceId;
    std::string nDeviceType;
    std::string nDeviceName;
    std::string nManufacture;
    std::string nAlias;
    std::string nRoom;
}; /* struct OCFDeviceTableInfo */

class HBDatabase;
class OCFDeviceTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    OCFDeviceTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~OCFDeviceTable();

    bool updateOrInsert(const OCFDeviceTableInfo &info);
    bool deleteBy(const OCFDeviceTableInfo &info, const std::string &where);
    bool queryBy(std::vector<OCFDeviceTableInfo> &infos, const OCFDeviceTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class OCFDeviceTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __OCFDeviceTable_H__ */
