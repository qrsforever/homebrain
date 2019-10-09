/* *****************************************************************
 *
 * Copyright 2018 Letv
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

#include "iotivity_config.h"

#include <mutex>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <climits>

#include "CommonApi.h"
#include "HBDeviceManager.h"
#include "HBDeviceManagerLog.h"
#include "OCFDeviceTable.h"

#define UNUSED_PARAMETER(P)       (void)(P)
using namespace HB;

namespace OIC {
namespace Service {
namespace HB {

HBDeviceManager* HBDeviceManager::m_instance = NULL;
HBDeviceManager::Garbage HBDeviceManager::m_garbage;
extern std::map<std::string, OCFDevice::Ptr> g_OCFDeviceList;
extern std::vector<OCFDeviceTableInfo> g_ownedDeviceList;

HBDeviceManager::HBDeviceManager()
{
    DM_LOGI("HBDeviceManager()\n");
    m_client = OCFClient::GetInstance();
}

HBDeviceManager::~HBDeviceManager()
{
    DM_LOGI("~HBDeviceManager()\n");
    std::cout << "~HBDeviceManager()" << std::endl;
    m_callback = nullptr;
    m_client = nullptr;
}

HBDeviceManager* HBDeviceManager::GetInstance()
{
    if (m_instance == NULL)
        m_instance = new HBDeviceManager();

    return m_instance;
}

int HBDeviceManager::Init()
{
    DM_LOGI("Init()\n");
    if (m_client != nullptr) {
        m_client->SetCallback(this);
        m_client->Init();
    }
    return 0;
}

int HBDeviceManager::Deinit()
{
    DM_LOGI("Deinit()\n");
    if (m_client != nullptr) {
        m_client->Deinit();
    }
    return 0;
}

int HBDeviceManager::DiscoverDevices()
{
    DM_LOGI("DiscoverDevices()\n");
    if (m_client != nullptr) {
        if (IPCA_OK == m_client->DiscoverDevices())
            return 0;
    }
    return -1;
}

int HBDeviceManager::GetAllDevices(std::map<std::string, OCFDevice::Ptr>& devices)
{
    if (g_OCFDeviceList.size() == 0)
        return 0;

    devices = g_OCFDeviceList;
    return 0;
}

int HBDeviceManager::GetDeviceList(std::map<std::string, std::string>& deviceList)
{
    std::map<std::string, OCFDevice::Ptr>::iterator it;
    if (g_OCFDeviceList.size() == 0)
        return 0;

    for (it = g_OCFDeviceList.begin(); it != g_OCFDeviceList.end(); it++) {
        OCFDevice::Ptr device = it->second;
        if (device->m_deviceType.empty())
            continue;

        deviceList[it->first] = device->m_deviceType;
    }
    return 0;
}

int HBDeviceManager::GetOwnedDeviceList(std::map<std::string, std::string>& deviceList)
{
    if (g_ownedDeviceList.size() == 0)
        return 0;

    for (int i = 0; i < g_ownedDeviceList.size(); i++) {
        OCFDeviceTableInfo info = g_ownedDeviceList[i];
        if (info.nDeviceType.empty())
            continue;

        deviceList[info.nDeviceId] = info.nDeviceType;
    }
    return 0;
}

int HBDeviceManager::GetDevicesTypeById(const std::string deviceId, std::string& deviceType)
{
    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (device != nullptr) {
        deviceType = device->m_deviceType;
        DM_LOGD("deviceId: %s, deviceType: %s\n", deviceId.c_str(), deviceType.c_str());
        return 0;
    }
    return -1;
}

int HBDeviceManager::GetDevicePropertyList(
        const std::string deviceId,
        std::map<std::string, HBPropertyType>& propertyList)
{
    if (m_client != nullptr) {
        std::map<std::string, IPCAValueType> properties;
        if (IPCA_OK != m_client->GetDeviceProperties(deviceId, properties))
            return -1;

        std::map<std::string, IPCAValueType>::iterator it;
        for (it = properties.begin(); it != properties.end(); it++) {
            switch (it->second) {
                case IPCA_INTEGER:
                    propertyList[it->first] = HB_PROPERTY_TYPE_INT;
                    break;
                case IPCA_BOOLEAN:
                    propertyList[it->first] = HB_PROPERTY_TYPE_BOOL;
                    break;
                case IPCA_STRING:
                    propertyList[it->first] = HB_PROPERTY_TYPE_STRING;
                    break;
                case IPCA_DOUBLE:
                    propertyList[it->first] = HB_PROPERTY_TYPE_DOUBLE;
                    break;
                default:
                    std::cout << "unsupported value type:" << it->second << std::endl;
                    break;
            }
            DM_LOGD("propertyKey: %s, type: %d\n", (it->first).c_str(), it->second);
        }
    }

    return 0;
}

int HBDeviceManager::GetDevicePropertyListWithValue(
        const std::string deviceId,
        std::map<std::string, std::string>& propertyList)
{
    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (device != nullptr) {
        propertyList = device->m_propertyList;
        return 0;
    }
    return -1;
}

int HBDeviceManager::SetDevicePropertyListWithValue(
        const std::string deviceId,
        std::map<std::string, std::string>& propertyList,
        bool async)
{
    if (m_client != nullptr) {
        if (IPCA_OK != m_client->SetDeviceProperties(deviceId, propertyList, async))
            return -1;
    }
    DM_LOGD("success.\n");
    return 0;
}

int HBDeviceManager::SetDevicePropertyValue(
        const std::string deviceId,
        const std::string propertyKey,
        const std::string value, bool async)
{
    DM_LOGD("deviceId: %s, propertyKey: %s, value: %s\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
    if (m_client != nullptr) {
        if (IPCA_OK != m_client->SetDeviceProperty(deviceId, propertyKey, value, async))
            return -1;
    }
    DM_LOGD("success.\n");
    return 0;
}

int HBDeviceManager::GetDevicePropertyValue(
        const std::string deviceId,
        const std::string propertyKey,
        std::string& value, bool async)
{
    //FIXME: don't support async get device property currently.
    UNUSED_PARAMETER(async);
    if (m_client != nullptr) {
        if (IPCA_OK != m_client->GetDevicePropertyValue(deviceId, propertyKey, value))
            return -1;
    }
    DM_LOGD("success. deviceId: %s, propertyKey: %s, value: %s\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
    return 0;
}

int HBDeviceManager::GetGatewayStatus(
        std::string gatewayId,
        HBGatewayStatus& status)
{
    status = HB_GATEWAY_STATUS_UNCONNECTED;

    if (m_client != nullptr) {
        OCFDevice::Ptr gateway = m_client->GetGatewayById(gatewayId);
        if (gateway != nullptr) {
            status = HB_GATEWAY_STATUS_CONNECTED;
            if (gateway->m_online == 1) {
                status = HB_GATEWAY_STATUS_READY;
            }
        }
    }
    DM_LOGD("gatewayId: %s, status: %d\n", gatewayId.c_str(), status);
    return 0;
}

int HBDeviceManager::EnableGatewayNet(std::string gatewayId)
{
    if (m_client != nullptr) {
        OCFDevice::Ptr gateway = m_client->GetGatewayById(gatewayId);
        if (gateway != nullptr) {
            if (IPCA_OK == m_client->SetDeviceProperty(gateway->m_deviceId, "trigger_net", "1", true)) {
                DM_LOGD("success. gatewayId: %s\n", gatewayId.c_str());
                return 0;
            }
        }
    }
    DM_LOGD("failed. gatewayId: %s\n", gatewayId.c_str());
    return -1;
}

int HBDeviceManager::RefreshOnlineStatus(std::string gatewayId)
{
    if (m_client != nullptr) {
        OCFDevice::Ptr gateway = m_client->GetGatewayById(gatewayId);
        if (gateway != nullptr) {
            if (IPCA_OK == m_client->SetDeviceProperty(gateway->m_deviceId, "refresh_online", "1", true)) {
                DM_LOGD("success. gatewayId: %s\n", gatewayId.c_str());
                return 0;
            }
        }
    }
    DM_LOGD("failed. gatewayId: %s\n", gatewayId.c_str());
    return -1;
}

int HBDeviceManager::DeleteDevice(std::string deviceId)
{
    std::string gatewayId;
    if (m_client != nullptr) {
        OCFDevice::Ptr device = GetDeviceById(deviceId);
        std::string uri = device->m_resourceUri;
        int cnt = 0;

        for (int i = 0; i < uri.length(); i++) {
            if (uri[i] == '/') {
                cnt++;
            } else if (cnt == 3) {
                gatewayId.push_back(uri[i]);
            }
        }

        m_client->DeleteDevice(deviceId);
        OCFDevice::Ptr gateway = m_client->GetGatewayById(gatewayId);
        if (device != nullptr) {
            if (IPCA_OK == m_client->SetDeviceProperty(gateway->m_deviceId, "delete_device", deviceId.c_str(), true)) {
                DM_LOGD("success. deviceId: %s, gatewayId: %s\n", deviceId.c_str(), gatewayId.c_str());
                return 0;
            }
        }
    }
    DM_LOGD("failed. deviceId: %s, gatewayId:%s\n", deviceId.c_str(), gatewayId.c_str());
    return -1;
}

int HBDeviceManager::GetDeviceOwnedStatus(
        std::string deviceId,
        HBDeviceOwnedStatus& ownedStatus)
{
    if (m_client != nullptr) {
        OCFDeviceOwnedStatus ocfOwnedStatus;
        IPCAStatus status = m_client->GetDeviceOwnedStatus(deviceId, ocfOwnedStatus);
        ownedStatus = static_cast<HBDeviceOwnedStatus>(ocfOwnedStatus);
        return (status == IPCA_OK) ? 0 : -1;
    }
    return -1;
}

int HBDeviceManager::SetDeviceOwnedStatus(
        std::string deviceId,
        HBDeviceOwnedStatus ownedStatus)
{
    if (m_client != nullptr) {
        IPCAStatus status = m_client->SetDeviceOwnedStatus(deviceId, static_cast<OCFDeviceOwnedStatus>(ownedStatus));
        return (status == IPCA_OK) ? 0 : -1;
    }
    return -1;
}

int HBDeviceManager::GetDeviceOnlineStatus(
        std::string deviceId,
        bool& online)
{
    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (device != nullptr) {
        if (device->m_online == 1)
            online = true;
        else
            online = false;
        DM_LOGD("deviceId: %s, isOnline: %d\n", deviceId.c_str(), online);
        return 0;
    }
    return -1;
}

int HBDeviceManager::SetCallback(HBDeviceCallBackHandler* callback)
{
    if (callback == nullptr) {
        std::cout << "SetCallback(): can't set nullptr to callback!" << std::endl;
        return -1;
    }
    m_callback = callback;
    return 0;
}

void HBDeviceManager::onOCFDeviceStatusChanged(
        const std::string deviceId,
        const std::string deviceType,
        std::string status)
{
    if (m_callback != nullptr) {
        HBDeviceStatus hbStatus = HB_DEVICE_STATUS_UNINITIALIZED;
        if (status.compare("online") == 0)
            hbStatus = HB_DEVICE_STATUS_ONLINE;
        else if (status.compare("offline") == 0)
            hbStatus = HB_DEVICE_STATUS_OFFLINE;
        else if (status.compare("unkown_error") == 0)
            hbStatus = HB_DEVICE_STATUS_UNKOWN_ERROR;

        DM_LOGD("deviceId: %s, deviceType: %s, status: %s\n", deviceId.c_str(), deviceType.c_str(), status.c_str());
		m_callback->onDeviceStatusChanged(deviceId, deviceType, hbStatus);
    }
}

void HBDeviceManager::onOCFDevicePropertyChanged(
        const std::string deviceId,
        std::string propertyKey,
        std::string propertyValue)
{
    if (m_callback != nullptr) {
        DM_LOGD("deviceId: %s, propertyKey: %s, propertyValue: %s\n", deviceId.c_str(), propertyKey.c_str(), propertyValue.c_str());
		m_callback->onDevicePropertyChanged(deviceId, propertyKey, propertyValue);
    }
}

} /*OIC*/
} /*Servcie*/
} /*HB*/
