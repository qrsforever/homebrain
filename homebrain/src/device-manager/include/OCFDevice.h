/* *****************************************************************
 *
 * Copyright 2017 Microsoft
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
#ifndef DEVICE_MANAGER_OCFDEVICE_H_
#define DEVICE_MANAGER_OCFDEVICE_H_

#include <mutex>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <memory>
#include <functional>

#include "ipca.h"

namespace OIC {
namespace Service {
namespace HB {

typedef enum {
    UNBINDED = 0,
    LOCAL_BINDED,
    CLOUD_BINDED,
} OCFDeviceOwnedStatus;

// Key of map is resource path. Value is array of resource types.
typedef std::map<std::string, std::vector<std::string>> ResourceList;
// Key is resource URI, Values is array of property name & property value type pair.
typedef std::map<std::string, std::map<std::string, IPCAValueType>> ResourceProperties;
// Key is property key, Values is property value.
typedef std::map<std::string, std::string> PropertiesList;

/**
 * Each OCFDevice object represents a device discovered by IPCA.
 */
class OCFDevice
{
public:
    typedef std::shared_ptr<OCFDevice> Ptr;
    OCFDevice(IPCAAppHandle appHandle, std::string id, std::string interId);
    ~OCFDevice();
    IPCADeviceInfo* GetDeviceInfo();
    IPCAPlatformInfo* GetPlatformInfo();
    ResourceList GetResourceInfo();

    std::string m_deviceId;
    std::string m_deviceType;
    IPCADeviceHandle m_deviceHandle;      // from IPCAOpenDevice();
    IPCAHandle m_observeHandle;           // from IPCAObserveResource();
    std::string m_resourceUri;            // keep function resource uri
    ResourceProperties m_properties;
    PropertiesList m_propertyList;
    OCFDeviceOwnedStatus m_ownedStatus;
    int m_online;

private:
    IPCAStatus OpenDevice();

private:
    std::string m_deviceIdInter;
    IPCAAppHandle m_ipcaAppHandle;
    std::string m_hostAddress;
    IPCADeviceInfo* m_deviceInfo;         // valid between IPCAOpenDevice() and IPCACloseDevice().
    IPCAPlatformInfo* m_platformInfo;     // valid between IPCAOpenDevice() and IPCACloseDevice().
    ResourceList m_resourceList;
};

} /*OIC*/
} /*Servcie*/
} /*HB*/
#endif
