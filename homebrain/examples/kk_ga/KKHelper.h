/***************************************************************************
 *  KKHelper.h -
 *
 *  Created: 2019-09-27
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __KKHelper_H__
#define __KKHelper_H__

#include <string>
#include <map>

#ifdef __cplusplus

namespace HB {

typedef enum {
    DEVICE_TYPE_LIGHT_PORCH = 1,
    DEVICE_TYPE_LIGHT_CHANDELIER = 2,
    DEVICE_TYPE_LIGHT_BELT = 3,
    DEVICE_TYPE_LIGHT_SPOT = 4,
    DEVICE_TYPE_DOOR_CONTACT = 12,
    DEVICE_TYPE_SCENE = 13,
    DEVICE_TYPE_AIRCONDITIONER = 15,
    DEVICE_TYPE_GATEWAY = 99,
} DeviceType;

typedef enum {
    VAR_TYPE_STRING,
    VAR_TYPE_BOOLEAN,
    VAR_TYPE_INT,
    VAR_TYPE_UINT,
    VAR_TYPE_INT64,
    VAR_TYPE_UINT64,
    VAR_TYPE_DOUBLE,
} VarType;

class deviceCallBackHandler {
public:
		virtual ~deviceCallBackHandler() {} ;

		virtual void onDeviceStatusChanged(const std::string deviceId, const std::string deviceName, const std::string productKey,
                DeviceType dtype, const std::string room, const std::string status) = 0;
		virtual void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, const std::string value) = 0;
};

class KKClient
{
public:
    KKClient() { }
    ~KKClient() { }
    int getHostDeviceId(std::string& hostDeviceId) { return 0; }
    int init() { return 0; }
    int getDeviceList(std::map<std::string, DeviceType>& deviceList) { return 0; }
    int setDeviceProperty(const std::string deviceId, const std::string propertyKey, const std::string value) { return 0; }
    int getDeviceProperty(const std::string deviceId, const std::string propertyKey, std::string& value, VarType& type) { return 0; }
    int setCallback(deviceCallBackHandler* callback) { return 0; }
};

const char* KKBuildVersion();
const char* KKBuildDatetime();

KKClient& deviceManager();

void KKDumpInfo();

}

#endif /* __cplusplus */

#endif /* __KKHelper_H__ */
