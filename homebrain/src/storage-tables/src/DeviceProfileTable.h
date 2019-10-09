/***************************************************************************
 *  DeviceProfileTable.h -
 *
 *  Created: 2018-12-17 15:29:05
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __DeviceProfileTable_H__
#define __DeviceProfileTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct DeviceTableInfo {
    DeviceTableInfo()
        : nDeviceType(""), nSuperType(""), nDeviceVer("")
        , nDeviceManu(""), nScriptData(""), nJsonData("") { }
    std::string nDeviceType;
    std::string nSuperType;
    std::string nDeviceName;
    std::string nDeviceVer;
    std::string nDeviceManu;
    std::string nIconId;
    std::string nScriptData;
    std::string nJsonData;
}; /* struct DeviceTableInfo */

class HBDatabase;
class DeviceProfileTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    DeviceProfileTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~DeviceProfileTable();

    bool updateOrInsert(const DeviceTableInfo &info);
    bool deleteBy(const DeviceTableInfo &info, const std::string &where);
    bool queryBy(std::vector<DeviceTableInfo> &infos, const DeviceTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class DeviceProfileTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __DeviceProfileTable_H__ */
