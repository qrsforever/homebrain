/* ****************************************************************
 *
 * Copyright 2017 Letv All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
#ifndef DEVICE_MANAGER_HBDEVICE_MANAGER_H_
#define DEVICE_MANAGER_HBDEVICE_MANAGER_H_

#include <string>
#include <list>
#include <vector>
#include "OCFClient.h"

namespace OIC {
namespace Service {
namespace HB {

/**
 * HBDeviceManager provides APIs to manage the devices
 * 
 */

typedef enum {
    HB_DEVICE_STATUS_UNINITIALIZED = 0,
    HB_DEVICE_STATUS_INITIALIZING,
    HB_DEVICE_STATUS_ONLINE,
    HB_DEVICE_STATUS_OFFLINE,
    HB_DEVICE_STATUS_UNKOWN_ERROR,
} HBDeviceStatus;

typedef enum {
    HB_DEVICE_UNBINDED = 0,
    HB_DEVICE_LOCAL_BINDED,
    HB_DEVICE_CLOUD_BINDED,
} HBDeviceOwnedStatus;

typedef enum {
    HB_GATEWAY_STATUS_UNCONNECTED = 0,
    HB_GATEWAY_STATUS_CONNECTED,
    HB_GATEWAY_STATUS_READY,
} HBGatewayStatus;

typedef enum {
    HB_PROPERTY_TYPE_INT = 0,
    HB_PROPERTY_TYPE_STRING,
    HB_PROPERTY_TYPE_BOOL,
    HB_PROPERTY_TYPE_DOUBLE,
} HBPropertyType;

typedef std::pair<std::string, std::string> DeviceInfo;

class HBDeviceCallBackHandler {
public:
		virtual ~HBDeviceCallBackHandler() {} ;

		virtual void onDeviceStatusChanged(const std::string deviceId, const std::string deviceType, HBDeviceStatus status) = 0;
		virtual void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value) = 0;
};

class HBDeviceManager : public OCFDeviceCallBackHandler {
public:
    static HBDeviceManager* GetInstance();
    virtual ~HBDeviceManager();

    /**
     * API to Init
     *
     * @return 0:success -1:fail.
     */
    int Init();

    /**
     * API to Deinit
     *
     * @return 0:success -1:fail.
     */
    int Deinit();

    /**
     * API to discover devices.
     *
     * @return 0:success -1:fail.
     */
    int DiscoverDevices();

    /**
     * API to get the list of devices
     *
     * @param[out] std::map<std::string, OCFDevice::Ptr>& deviceList. key is device id.
     * @return 0:success -1:fail.
     */
    int GetAllDevices(std::map<std::string, OCFDevice::Ptr>& deviceList);

    /**
     * API to get the list of devices
     *
     * @param[out] std::map<std::string, std::string>& deviceList. key is device id. value is device type.
     * @return 0:success -1:fail.
     */
    int GetDeviceList(std::map<std::string, std::string>& deviceList);

    /**
     * API to get devices by type
     *
     * @param[in] std::string deviceType.
     * @param[out] std::map<std::string, OCFDevice::Ptr>& deviceList. key is device id.
     * @return 0:success -1:fail.
     */
    int GetDevicesByType(const std::string deviceType, std::map<std::string, OCFDevice::Ptr>& deviceList);

    /**
     * API to get deviceType by device id.
     *
     * @param[in] deviceId.
     * @param[out] deviceType.
     * @return 0:success -1:fail.
     */
    int GetDevicesTypeById(const std::string deviceId, std::string& deviceType);

    /**
     * API to get property list of device
     *
     * @param[in] deviceId.
     * @param[out] std::map<std::string, HBPropertyType>& propertyList. key is property key.
     * @return 0:success -1:fail.
     */
    int GetDevicePropertyList(const std::string deviceId, std::map<std::string, HBPropertyType>& propertyList);

    /**
     * API to get property list of device with the current value
     *
     * @param[in] deviceId.
     * @param[out] std::map<std::string, std::string>& propertyList. key is property key, value is the current property value.
     * @return 0:success -1:fail.
     */
    int GetDevicePropertyListWithValue(const std::string deviceId, std::map<std::string, std::string>& propertyList);

    /**
     * API to set property list of device with the specific values
     *
     * @param[in] deviceId.
     * @param[in] std::map<std::string, std::string>& propertyList. key is property key, value is the property value.
     * @return 0:success -1:fail.
     */
    int SetDevicePropertyListWithValue(const std::string deviceId, std::map<std::string, std::string>& propertyList, bool async);

    /**
     * API to set property of device
     *
     * @param[in] deviceId: device ID
     * @param[in] propertyKey: the property name
     * @param[in] value: the property value
     * @return 0:success -1:fail.
     */
    int SetDevicePropertyValue(const std::string deviceId, const std::string propertyKey, const std::string value, bool async);
    
    /**
     * API to get property value of device
     *
     * @param[in] deviceId: device ID
     * @param[in] propertyKey: the property name
     * @param[out] value: the property value
     * @return 0:success -1:fail.
     */
    int GetDevicePropertyValue(const std::string deviceId, const std::string propertyKey, std::string& value, bool async);

    /**
     * API to get gateway status
     *
     * @param[in] gatewayId: gateway ID
     * @param[out] status: HBGatewayStatus
     * @return 0:success -1:fail.
     */
    int GetGatewayStatus(std::string gatewayId, HBGatewayStatus& status);

    /**
     * API to enable gateway scan subdev.
     *
     * @param[in] gatewayId: gateway ID
     * @return 0:success -1:fail.
     */
    int EnableGatewayNet(std::string gatewayId);

    /**
     * API to delete subdev.
     *
     * @param[in] deviceId: device ID
     * @return 0:success -1:fail.
     */
    int DeleteDevice(std::string deviceId);

    /**
     * API to get device owned status.
     *
     * @param[in] deviceId: device ID
     * @param[in] ownedStatus: HBDeviceOwnedStatus
     * @return 0:success -1:fail.
     */
    int GetDeviceOwnedStatus(std::string deviceId, HBDeviceOwnedStatus& ownedStatus);

    /**
     * API to mark device owned status.
     *
     * @param[in] deviceId: device ID
     * @param[in] ownedStatus: HBDeviceOwnedStatus
     * @return 0:success -1:fail.
     */
    int SetDeviceOwnedStatus(std::string deviceId, HBDeviceOwnedStatus ownedStatus);

    /**
     * API to query device online status.
     *
     * @param[in] deviceId: device ID
     * @param[out] online: true or false
     * @return 0:success -1:fail.
     */
    int GetDeviceOnlineStatus(std::string deviceId, bool& online);

    /**
     * API to set callback function
     *
     * @param[in] callback
     * @return 0:success -1:fail.
     * notes: it must be called before init().
     */
    int SetCallback(HBDeviceCallBackHandler* callback);
protected:
	void onOCFDeviceStatusChanged(const std::string deviceId, const std::string deviceType, std::string status);
	void onOCFDevicePropertyChanged(const std::string deviceId, std::string propertyKey, std::string propertyValue);

private:
    HBDeviceManager();
    class Garbage {
        public:
            ~Garbage() {
                if (HBDeviceManager::m_instance != nullptr)
                    delete HBDeviceManager::m_instance;
            }
    };
private:
    OCFClient* m_client;
    HBDeviceCallBackHandler* m_callback;
    static Garbage m_garbage;
    static HBDeviceManager* m_instance;
};

} /*OIC*/
} /*Servcie*/
} /*HB*/
#endif /* DEVICE_MANAGER_HBDEVICE_MANAGER_H_ */
