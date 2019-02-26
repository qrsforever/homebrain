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

#include <unistd.h>
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
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "CommonApi.h"
#include "Common.h"
#include "OCFClient.h"
#include "HBDeviceManagerLog.h"

#define UNUSED_PARAMETER(P)       (void)(P)
#define MAX_TIME_OUT_MS    (3*1000)
#define MIN_TIME_OUT_MS    (1*1000)
#define DEVICE_DID_LENTH    23

//FIXME: temporary WR, remove this later
//#define TV_HEARTBEAT_CHECK_ONLINE 1

using namespace UTILS;
using namespace OC;

namespace OIC {
namespace Service {
namespace HB {
// Handles
IPCAAppHandle g_ipcaAppHandle = nullptr;
IPCAHandle g_discoverDeviceHandle = nullptr;

std::recursive_mutex g_globalMutex;   // Sync access to g_OCFDeviceList.
std::recursive_mutex g_contextsMutex;   // Sync access to g_contexts.
// Key is device id.  Value is OCFDevices.
std::map<std::string, OCFDevice::Ptr> g_OCFDeviceList;
std::vector<CallbackCtx*> g_contexts;

std::condition_variable g_setPropertiesCompleteCV;
std::condition_variable g_getPropertiesCompleteCV;
std::condition_variable g_getPropertyValueCompleteCV;
std::string g_propertyValue;

OCFClient* OCFClient::m_instance = NULL;
OCFClient::Garbage OCFClient::m_garbage;

#ifdef TV_HEARTBEAT_CHECK_ONLINE 
long int g_pre_heartbeat = 0;
long int g_count=0;
long int g_heartbeat = 0;
bool g_tvpower = false;

void* HeartbeatThread (void* para)
{
    while (1) {
        if (g_pre_heartbeat != g_heartbeat) {
            g_pre_heartbeat = g_heartbeat;
            g_count=0;
        }
        else
            g_count++;

        if(g_count>3 && g_tvpower)
        {
            OCFClient* client = OCFClient::GetInstance();
            if (client == nullptr) {
                std::cout << "failed to OCFClient GetInstance." << std::endl;
                return nullptr;
            }
            OCFDeviceCallBackHandler* pCallback = client->GetCallback();
            if (pCallback == nullptr) {
                std::cout << "failed to OCFClient GetCallback." << std::endl;
                return nullptr;
            }

            pCallback->onOCFDeviceStatusChanged("d361683c-03bd-9d56-75ed", "oic.d.letv", "offline");
            g_tvpower = false;
            std::cout << "TV Power off>>>>>>>>>>>>>>>>>>>" << std::endl;
        }
        sleep(1);
    }
}
#endif

OCFDevice::Ptr GetDeviceById(std::string deviceId)
{
    std::lock_guard<std::recursive_mutex> lock(g_globalMutex);
    if (g_OCFDeviceList.find(deviceId) == g_OCFDeviceList.end()) {
        std::cout << "can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return nullptr;
    }

    return g_OCFDeviceList[deviceId];
}

// Callback from IPCAObserveResource().
void IPCA_CALL ResourceChangeNotificationCallback(
                                        IPCAStatus result,
                                        void* context,
                                        IPCAPropertyBagHandle propertyBagHandle)
{
    OCFClient* client = OCFClient::GetInstance();
    if (!client) {
        std::cout << "failed to OCFClient GetInstance." << std::endl;
        return;
    }

    std::string* deviceId = reinterpret_cast<std::string*>(context);
    if (result != IPCA_OK) {
        std::cout << "!!! ResourceChangeNotificationCallback callback !!! error: " << result
            << std::endl;
        if (result == IPCA_DEVICE_APPEAR_OFFLINE) {
            LOGW("ResourceChangeNotificationCallback(): device appears offline. deviceId: %s\n", (*deviceId).c_str());
            OCFDeviceCallBackHandler* pCallback = client->GetCallback();
            if (pCallback != nullptr) {
                pCallback->onOCFDeviceStatusChanged(*deviceId, "", "unkown_error");
            }
            //client->DiscoverDevices();
        }
        return;
    }

#ifdef TV_HEARTBEAT_CHECK_ONLINE 
    OCFDevice::Ptr device = GetDeviceById(*deviceId);
    if (device->m_deviceId.compare("d361683c-03bd-9d56-75ed") == 0) {
        g_heartbeat++;
        return;
    }
#endif
    if (propertyBagHandle != nullptr) {
        client->NotifyDeviceProperties(*deviceId, propertyBagHandle);
    }

    return;
}

// Callback when device is discovered.
void IPCA_CALL DiscoverDevicesCallback(
        void* context,
        IPCADeviceStatus deviceStatus,
        const IPCADiscoveredDeviceInfo* deviceInfo)
{
    UNUSED_PARAMETER(context);

    std::string deviceIdInter = deviceInfo->deviceId;
    std::string deviceId;
    deviceId.assign(deviceIdInter, 0, DEVICE_DID_LENTH);

    {
        std::lock_guard<std::recursive_mutex> lock(g_globalMutex);

        if (g_OCFDeviceList.find(deviceId) == g_OCFDeviceList.end()) {
            OCFDevice::Ptr newDevice = std::shared_ptr<OCFDevice>(new
                    OCFDevice(g_ipcaAppHandle, deviceId, deviceIdInter));
            if (!newDevice) {
                std::cout << "Out of memory" << std::endl;
                return;
            }
            g_OCFDeviceList[deviceId] = newDevice;
            std::cout << "#######add new device: " << deviceId << std::endl;
        }
    }
    OCFDevice::Ptr ocfDevice = g_OCFDeviceList[deviceId];
    OCFClient* client = OCFClient::GetInstance();
    if (!client) {
        std::cout << "failed to get OCFClient!" << std::endl;
        return;
    }

    OCFDeviceCallBackHandler* pCallback = client->GetCallback();
    // handle IPCA device status
    if (deviceStatus == IPCA_DEVICE_DISCOVERED) {
        std::cout << "*** New device: *** " << std::endl << std::endl;
    }
    else if (deviceStatus == IPCA_DEVICE_UPDATED_INFO) {
        std::cout << "+++ device info updated for id: [" << deviceId << "] +++";
        std::cout << std::endl << std::endl;
    } else {
        if (g_OCFDeviceList.find(deviceId) != g_OCFDeviceList.end()) {
            LOGW("--- device appears offline. Device ID: [%s]---remove this device.\n", deviceId.c_str());
            if (pCallback != nullptr) {
                pCallback->onOCFDeviceStatusChanged(deviceId, ocfDevice->m_deviceType, "offline");
            }
            // FIXME: device no longer exist, no need to notify server 
            /*if (device->m_observeHandle != nullptr)
              IPCACloseHandle(device->m_observeHandle, nullptr, 0);*/
            g_OCFDeviceList.erase(deviceId);
        }
        return;
    }

    // try to get device info
    std::vector<std::string> deviceUris;
    IPCADeviceInfo* di = ocfDevice->GetDeviceInfo();

    for (size_t i = 0; i < deviceInfo->deviceUriCount; i++) {
        deviceUris.push_back(deviceInfo->deviceUris[i]);
    }

    if (di != nullptr) {
        std::cout << "Device Info: "  << std::endl;
        std::cout << std::endl;
        if (deviceUris.size() != 0) {
            int i = 0;
            for (auto& uri : deviceUris) {
                if (i++ == 0) {
                    std::cout << "   Device URI . . . . . . . :  " << uri << std::endl;
                } else {
                    std::cout << "                               " << uri << std::endl;
                }
            }
        }

        std::cout << "   Device Internal ID  . . . . . . . :  " << (di->deviceId ? di->deviceId : "");
        std::cout << std::endl;
        std::cout << "   Device Name  . . . . . . :  ";
        std::cout << (di ->deviceName ? di->deviceName : "") << std::endl;
        //std::cout << "   Device Software Version  :  ";
        //std::cout << (di->deviceSoftwareVersion ? di->deviceSoftwareVersion : "") << std::endl;
        std::cout << std::endl;
    }
    else {
        std::cout << "Device Info: Not available."  << std::endl << std::endl;
    }

    // try to get resource uri.
    if (ocfDevice->m_resourceUri.empty())
        ocfDevice->GetResourceInfo();

    // try to get device properties
    if (!ocfDevice->m_resourceUri.empty()) {
        std::map<std::string, IPCAValueType> knownProperties = ocfDevice->m_properties[ocfDevice->m_resourceUri];
        if ((knownProperties.size() == 0) &&
                (IPCA_OK != client->GetDeviceProperties_Impl(deviceId))) {
            LOGW("GetDeviceProperties failed! deviceId: %s\n", deviceId.c_str());
        }
    }

    // try to get device online status
    //FIXME:
    // Currently we can only get the online state at the beginning.
    // one letv can be thought online when this letv is discovered.
    if (ocfDevice->m_online == -1) {
    std::string value;
    if (0 == DEVICE_TYPE::LE_TV.compare(ocfDevice->m_deviceType)) {
            if (pCallback != nullptr)
                pCallback->onOCFDeviceStatusChanged(deviceId, ocfDevice->m_deviceType, "online");
            ocfDevice->m_online = 1;
    } else {
        if (0 == client->GetDevicePropertyValue(deviceId, "online", value)) {
            if (value.compare("1") == 0) {
                ocfDevice->m_online = 1;
                pCallback->onOCFDeviceStatusChanged(deviceId, ocfDevice->m_deviceType, "online");
            } else
                ocfDevice->m_online = 0;
        }
    }
    }

    // request observe
    if (1 == ocfDevice->m_online && !ocfDevice->m_observeHandle) {
        if(IPCA_OK != client->RequestObserve(deviceId))
            std::cout << "failed to request observe for device: " << deviceId << std::endl;
        DM_LOGD("RequestObserve sucess. deviceId: %s, uri: %s\n", deviceId.c_str(),ocfDevice->m_resourceUri.c_str());
    }
    /*FIXME: if observer is removed by server, we need re-request observer*/
#if 0
    else if ((0 == client->GetDevicePropertyValue(deviceId, "observers", value)) &&
            (value.compare("0") == 0)) {
        ocfDevice->m_observeHandle = nullptr;
        if(IPCA_OK != client->RequestObserve(deviceId))
            std::cout << "failed to re-request observe for device: " << deviceId << std::endl;
        DM_LOGD("observer is removed by server, re-request observer for device: %s\n", deviceId.c_str());
    }
#endif
    return;
}

void ListenDevicesCallback(OCStackResult result, const unsigned int nonce, const std::string& hostAddress)
{
    std::cout << "Received presence notification from : " << hostAddress << std::endl;
    std::cout << "In presenceHandler: ";

    OCFClient* client = OCFClient::GetInstance();
    if (!client) {
        std::cout << "failed to OCFClient GetInstance." << std::endl;
        return;
    }

    client->DiscoverDevices();

    switch(result)
    {
        case OC_STACK_OK:
            std::cout << "Nonce# " << nonce << std::endl;
            break;
        case OC_STACK_PRESENCE_STOPPED:
            std::cout << "Presence Stopped\n";
            break;
        case OC_STACK_PRESENCE_TIMEOUT:
            std::cout << "Presence Timeout\n";
            break;
        default:
            std::cout << "Error\n";
            break;
    }
}

// IPCASetProperties() completion callback.
void IPCA_CALL SetPropertiesCallback(
                        IPCAStatus result,
                        void* context,
                        IPCAPropertyBagHandle propertyBagHandle)
{
    UNUSED_PARAMETER(propertyBagHandle);
    CallbackCtx* ctx = reinterpret_cast<CallbackCtx*>(context);

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        unsigned int i = 0;
        for (; i < g_contexts.size(); i++) {
            if (g_contexts[i] == ctx) {
                break;
            }
        }

        if (i == g_contexts.size()) {
            std::cout << "SetPropertiesCallback(): context has been delete, do nothing." << std::endl;
            return;
        }
    }

    if (result == IPCA_OK) {
        std::cout << "SetPropertiesCallback(): successful." << std::endl;
    }
    else {
        std::cout << "SetPropertiesCallback(): error: " << result << std::endl;
    }
    g_setPropertiesCompleteCV.notify_all();
}

// IPCAGetProperties() completion callback.
void IPCA_CALL GetPropertiesCallback(
                            IPCAStatus result,
                            void* context,
                            IPCAPropertyBagHandle propertyBagHandle)
{
    IPCAStatus status;
    CallbackCtx* ctx = reinterpret_cast<CallbackCtx*>(context);
    //char* resourcePath;
    char** allKeys;
    IPCAValueType* allValueTypes;
    size_t count;

    std::cout << "GetPropertiesCallback(): entering " << std::endl;
    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        unsigned int i = 0;
        for (; i < g_contexts.size(); i++) {
            if (g_contexts[i] == ctx) {
                break;
            }
        }

        if (i == g_contexts.size()) {
            std::cout << "GetPropertiesCallback(): context has been delete, do nothing." << std::endl;
            return;
        }
    }

    if (result != IPCA_OK) {
        std::cout << "GetPropertiesCallback(): error: " << result << std::endl;
        g_getPropertiesCompleteCV.notify_all();
        return;
    }

    /*status = IPCAPropertyBagGetResourcePath(propertyBagHandle, &resourcePath);
    std::cout << "resourcePath = " << resourcePath << std::endl;
    if ((status != IPCA_OK) || (resourcePath == nullptr)) {
        return;
    }*/

    status = IPCAPropertyBagGetAllKeyValuePairs(propertyBagHandle,
                            &allKeys, &allValueTypes, &count);
    if (status != IPCA_OK) {
        //IPCAPropertyBagFreeString(resourcePath);
        std::cout << "GetPropertiesCallback(): failed to IPCAPropertyBagGetAllKeyValuePairs" << std::endl;
        g_getPropertiesCompleteCV.notify_all();
        return;
    }

    std::map<std::string, IPCAValueType> properties;
    for (size_t i = 0 ; i < count ; i++) {
        properties[allKeys[i]] = allValueTypes[i];
        std::cout << "GetPropertiesCallback(): property_key: " << allKeys[i]
            <<", type: "<< allValueTypes[i] << std::endl;
    }

    OCFDevice::Ptr device = GetDeviceById(ctx->deviceId);

    if (!device) {
        std::cout << "GetPropertiesCallback(): can't find device for deviceId:" << (ctx->deviceId).c_str() << std::endl;
        g_getPropertiesCompleteCV.notify_all();
        return;
    }

    /*if (device->m_resourceUri.compare(resourcePath) != 0) {
        strncpy(resourcePath, device->m_resourceUri.c_str(), device->m_resourceUri.length());
        resourcePath[device->m_resourceUri.length()] = '\0';
        std::cout << "resource path error! m_resourceUri = " << device->m_resourceUri.c_str() << ", resource path = " << resourcePath << std::endl;
    }*/

    std::map<std::string, IPCAValueType> knownProperties = device->m_properties[device->m_resourceUri];
    for (auto& newProperty : properties) {
        if ((knownProperties.size() == 0) ||
            (knownProperties.find(newProperty.first) == knownProperties.end())) {
            // At least one new property is not in known properties.
            // Replace known properties & display device.
            device->m_properties[device->m_resourceUri.c_str()] = properties;
            std::cout << "GetPropertiesCallback(): === Updated info on device properties: === " << std::endl << std::endl;
            OCFClient* client = OCFClient::GetInstance();
            if (client != nullptr) {
                client->NotifyDeviceProperties(ctx->deviceId, propertyBagHandle);
            }
            break;
        }
    }

    //IPCAPropertyBagFreeString(resourcePath);
    IPCAPropertyBagFreeStringArray(allKeys, count);
    IPCAPropertyBagFreeIPCAValueTypeArray(allValueTypes);
   
    g_getPropertiesCompleteCV.notify_all();
}

void IPCA_CALL GetPropertyValueCallback(
                            IPCAStatus result,
                            void* context,
                            IPCAPropertyBagHandle propertyBagHandle)
{
    CallbackCtx* ctx = reinterpret_cast<CallbackCtx*>(context);
    std::cout << "=====GetPropertyValueCallback(): entering: ctx = " << context << std::endl;
    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        unsigned int i = 0;
        for (; i < g_contexts.size(); i++) {
            if (g_contexts[i] == ctx) {
                break;
            }
        }

        if (i == g_contexts.size()) {
            std::cout << "GetPropertyValueCallback(): context has been delete, do nothing." << std::endl;
            return;
        }
    }

    if (result != IPCA_OK) {
        std::cout << "GetPropertyValueCallback(): error: " << result << std::endl;
        g_getPropertyValueCompleteCV.notify_all();
        return;
    }

    std::string deviceId(ctx->deviceId);
    std::string propertyKey(ctx->propertyKey);

    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "GetPropertyValueCallback(): can't find device for deviceId:"
            << deviceId.c_str() << std::endl;
        g_getPropertyValueCompleteCV.notify_all();
        return;
    }


    if (propertyBagHandle != nullptr) {
        std::map<std::string, IPCAValueType> properties = device->m_properties[device->m_resourceUri];
        if (properties.find(propertyKey) == properties.end()) {
            g_getPropertyValueCompleteCV.notify_all();
            std::cout << "GetPropertyValueCallback(): failed to get property " << propertyKey << std::endl;
            return;
        }
        IPCAValueType type = properties[propertyKey];
        switch (type) {
            case IPCA_INTEGER:
                int intValue;
                IPCAPropertyBagGetValueInt(propertyBagHandle, propertyKey.c_str(), &intValue);
                g_propertyValue = int2String(intValue);
                break;
            case IPCA_BOOLEAN:
                bool boolValue;
                IPCAPropertyBagGetValueBool(propertyBagHandle, propertyKey.c_str(), &boolValue);
                g_propertyValue = int2String(boolValue);
                break;
            case IPCA_STRING:
                char *stringValue;
                IPCAPropertyBagGetValueString(propertyBagHandle, propertyKey.c_str(), &stringValue);
                g_propertyValue.assign(stringValue);
                break;
            case IPCA_DOUBLE:
                double doubleValue;
                IPCAPropertyBagGetValueDouble(propertyBagHandle, propertyKey.c_str(), &doubleValue);
                g_propertyValue = double2String(doubleValue);
                break;
            default:
                std::cout << "unsupported value type:" << type << std::endl;
                break;
        }
    }

    std::cout << "GetPropertyValueCallback(): successful." << std::endl;

    g_getPropertyValueCompleteCV.notify_all();
}

OCFClient::OCFClient() :
    m_discoverDeviceHandle(nullptr),
    m_callback(nullptr),
    m_initialized(false)
{
    g_OCFDeviceList.clear();
    g_propertyValue.clear();
    g_contexts.clear();
}

OCFClient::~OCFClient()
{
    std::cout << "~OCFClient()" << std::endl;
    g_OCFDeviceList.clear();
    g_propertyValue.clear();
    g_contexts.clear();
    m_discoverDeviceHandle = nullptr;
    m_callback = nullptr;
}

OCFClient* OCFClient::GetInstance()
{
    if (m_instance == NULL)
        m_instance = new OCFClient();

    return m_instance;
}

IPCAStatus OCFClient::Init()
{
    IPCAUuid appId = {
                       {0xb6, 0x12, 0x38, 0x0c, 0x8c, 0x4c, 0x11, 0xe6,
                        0xae, 0x22, 0x56, 0xb6, 0xb6, 0x49, 0x96, 0x11}
                     };

    IPCAAppInfo ipcaAppInfo = { appId, "HB_Client", "1.0.0", "LeTV" };

    IPCAStatus status = IPCA_OK;
    if (m_initialized) {
        std::cout << "OCFClient has already been initialized." << std::endl;
        return status;
    }

    status = IPCAOpen(&ipcaAppInfo, IPCA_VERSION_1, &g_ipcaAppHandle);
    if (status != IPCA_OK) {
        std::cout << "Failed IPCAOpen(). Status: " << status << std::endl;
        return status;
    }

    status = DiscoverDevices();
    if (status != IPCA_OK) {
        std::cout << "failed to discover devices." << std::endl;
    }

    //FIXME: don't know how many devices need to be discovered.
    usleep(5000*1000);

    ListenDevices();
#ifdef TV_HEARTBEAT_CHECK_ONLINE 
    //FIXME: heart beat thread to listen TV's offline.
    pthread_t threadId;
    static int startedThread = 0;

    // Observation happens on a different thread in ChangeTvRepresentation function.
    // If we have not created the thread already, we will create one here.
    if(!startedThread)
    {
        pthread_create (&threadId, NULL, HeartbeatThread, NULL);
        startedThread = 1;
    }
#endif

    m_initialized = true;
    return status;
}

IPCAStatus OCFClient::Deinit()
{
    if (m_initialized) {
        if (g_discoverDeviceHandle != nullptr) {
            IPCACloseHandle(g_discoverDeviceHandle, nullptr, 0);
        }

        std::map<std::string, OCFDevice::Ptr>::iterator it;
        for (it = g_OCFDeviceList.begin(); it != g_OCFDeviceList.end(); it++) {
            OCFDevice::Ptr device = it->second;
            if (device->m_observeHandle != nullptr)
                IPCACloseHandle(device->m_observeHandle, nullptr, 0);
        }
        g_OCFDeviceList.clear();

        if (g_ipcaAppHandle != nullptr) {
            IPCAClose(g_ipcaAppHandle);
            g_ipcaAppHandle = nullptr;
        }
        g_propertyValue.clear();
        g_contexts.clear();
        m_initialized = false;
    }
    return IPCA_OK;
}

IPCAStatus OCFClient::SetDeviceProperty(
        std::string deviceId,
        std::string propertyKey,
        std::string propertyValue, bool async)
{
    std::unique_lock<std::mutex> set_lock { m_setPropertiesCbMutex };
    IPCAStatus status = IPCA_OK;
    IPCAPropertyBagHandle propertyBagHandle;
    CallbackCtx* ctx;

    std::cout << "====SetDeviceProperty(): entering: propertykey = " << propertyKey.c_str()
        << ", value = "<< propertyValue.c_str() << std::endl;
    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "SetDeviceProperty(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    if (device->m_online != 1) {
        std::cout << "SetDeviceProperty(): device is offline. don't allow to set! deviceID =" <<
            deviceId<< std::endl;
        return IPCA_FAIL;
    }

    status = IPCAPropertyBagCreate(&propertyBagHandle);
    if (IPCA_OK != status)
    {
        std::cout << "failed to IPCAPropertyBagCreate" << std::endl;
        return IPCA_FAIL;
    }
 
    std::map<std::string, IPCAValueType> properties = device->m_properties[device->m_resourceUri];
    if (properties.find(propertyKey) == properties.end()) {
        std::cout << "SetDeviceProperty(): failed to get property " << propertyKey.c_str() << std::endl;
        return IPCA_FAIL;
    }

    IPCAValueType type = properties[propertyKey];
    int intValue;
    bool boolValue;
    double doubleValue;
    switch (type) {
        case IPCA_INTEGER:
            intValue = atoi(propertyValue.c_str());
            status = IPCAPropertyBagSetValueInt(propertyBagHandle,
                    propertyKey.c_str(),
                    intValue);
            break;
        case IPCA_BOOLEAN:
            boolValue = (atoi(propertyValue.c_str()) == 0) ? false : true;
            status = IPCAPropertyBagSetValueBool(propertyBagHandle,
                    propertyKey.c_str(),
                    boolValue);
            break;
        case IPCA_STRING:
            status = IPCAPropertyBagSetValueString(propertyBagHandle,
                    propertyKey.c_str(),
                    propertyValue.c_str());
            break;
        case IPCA_DOUBLE:
            doubleValue = atof(propertyValue.c_str());
            status = IPCAPropertyBagSetValueDouble(propertyBagHandle,
                    propertyKey.c_str(),
                    doubleValue);
            break;
        default:
            std::cout << "unsupported value type:" << type << std::endl;
            break;
    }

    if (IPCA_OK != status)
    {
        std::cout << "SetDeviceProperty(): failed to IPCAPropertyBagSetValue" << std::endl;
        return IPCA_FAIL;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        ctx = new CallbackCtx;
        if (!ctx) {
            std::cout << "failed to new CallbackCtx." << std::endl;
            return IPCA_FAIL;
        }

        g_contexts.push_back(ctx);
    }
    status = IPCASetProperties(device->m_deviceHandle,
            &SetPropertiesCallback,
            reinterpret_cast<void*>(ctx),
            device->m_resourceUri.c_str(),
            nullptr,
            nullptr,
            propertyBagHandle,
            nullptr);

    if (IPCA_OK == status && !async) {
        if ( std::cv_status::timeout == g_setPropertiesCompleteCV.wait_for(
                set_lock, std::chrono::milliseconds{ MIN_TIME_OUT_MS })) {
            status = IPCA_REQUEST_TIMEOUT;
            std::cout << "SetDeviceProperty(): time out! deviceId =" << deviceId.c_str()
                << ", property_key = " << propertyKey.c_str() << std::endl;
            goto exit;
        }
    }

exit:
    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        std::vector<CallbackCtx*>::iterator it;
        for (it = g_contexts.begin(); it != g_contexts.end(); it++) {
            if (ctx == *it) {
                std::cout << "SetDeviceProperty(): delete ctx = " << ctx << std::endl;
                delete ctx;
                g_contexts.erase(it);
                break;
            }
        }
    }
    IPCAPropertyBagDestroy(propertyBagHandle);
    return status;
}

IPCAStatus OCFClient::SetDeviceProperties(
        std::string deviceId,
        std::map<std::string, std::string>& propertyList,
        bool async)
{
    std::unique_lock<std::mutex> set_lock { m_setPropertiesCbMutex };
    IPCAStatus status = IPCA_OK;
    IPCAPropertyBagHandle propertyBagHandle;
    CallbackCtx* ctx;

    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "SetDeviceProperties(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    if (device->m_online != 1) {
        std::cout << "SetDeviceProperties(): device is offline. don't allow to set! deviceID =" <<
            deviceId<< std::endl;
        return IPCA_FAIL;
    }

    status = IPCAPropertyBagCreate(&propertyBagHandle);
    if (IPCA_OK != status)
    {
        std::cout << "failed to IPCAPropertyBagCreate" << std::endl;
        return IPCA_FAIL;
    }
 
    std::map<std::string, IPCAValueType> properties = device->m_properties[device->m_resourceUri];
    std::map<std::string, std::string>::iterator it;

    for (it = propertyList.begin(); it != propertyList.end(); it++) {
        std::string propertyKey = it->first;
        std::string propertyValue = it->second;
        if (properties.find(propertyKey) == properties.end()) {
            std::cout << "SetDeviceProperties(): failed to get property " << propertyKey.c_str() << std::endl;
            return IPCA_FAIL;
        }

        IPCAValueType type = properties[propertyKey];
        int intValue;
        bool boolValue;
        double doubleValue;
        switch (type) {
            case IPCA_INTEGER:
                intValue = atoi(propertyValue.c_str());
                status = IPCAPropertyBagSetValueInt(propertyBagHandle,
                        propertyKey.c_str(),
                        intValue);
                break;
            case IPCA_BOOLEAN:
                boolValue = (atoi(propertyValue.c_str()) == 0) ? false : true;
                status = IPCAPropertyBagSetValueBool(propertyBagHandle,
                        propertyKey.c_str(),
                        boolValue);
                break;
            case IPCA_STRING:
                status = IPCAPropertyBagSetValueString(propertyBagHandle,
                        propertyKey.c_str(),
                        propertyValue.c_str());
                break;
            case IPCA_DOUBLE:
                doubleValue = atof(propertyValue.c_str());
                status = IPCAPropertyBagSetValueDouble(propertyBagHandle,
                        propertyKey.c_str(),
                        doubleValue);
                break;
            default:
                std::cout << "unsupported value type:" << type << std::endl;
                break;
        }
    }

    if (IPCA_OK != status)
    {
        std::cout << "SetDeviceProperties(): failed to IPCAPropertyBagSetValue" << std::endl;
        return IPCA_FAIL;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        ctx = new CallbackCtx;
        if (!ctx) {
            std::cout << "failed to new CallbackCtx." << std::endl;
            return IPCA_FAIL;
        }

        g_contexts.push_back(ctx);
    }
    status = IPCASetProperties(device->m_deviceHandle,
            &SetPropertiesCallback,
            reinterpret_cast<void*>(ctx),
            device->m_resourceUri.c_str(),
            nullptr,
            nullptr,
            propertyBagHandle,
            nullptr);

    if (IPCA_OK == status && !async) {
        if ( std::cv_status::timeout == g_setPropertiesCompleteCV.wait_for(
                set_lock, std::chrono::milliseconds{ MIN_TIME_OUT_MS })) {
            status = IPCA_REQUEST_TIMEOUT;
            goto exit;
        }
    }

exit:
    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        std::vector<CallbackCtx*>::iterator it;
        for (it = g_contexts.begin(); it != g_contexts.end(); it++) {
            if (ctx == *it) {
                std::cout << "SetDeviceProperties(): delete ctx = " << ctx << std::endl;
                delete ctx;
                g_contexts.erase(it);
                break;
            }
        }
    }
    IPCAPropertyBagDestroy(propertyBagHandle);
    return status;
}

IPCAStatus OCFClient::GetDeviceProperties_Impl(std::string deviceId)
{
    std::unique_lock<std::mutex> get_lock { m_getPropertiesCbMutex };

    IPCAStatus status = IPCA_OK;
    CallbackCtx* ctx;
    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "GetDeviceProperties_Impl(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        ctx = new CallbackCtx;
        if (!ctx) {
            std::cout << "failed to new CallbackCtx." << std::endl;
            return IPCA_FAIL;
        }

        ctx->deviceId = deviceId;
        g_contexts.push_back(ctx);
    }

    IPCAStatus getStatus = IPCAGetProperties(device->m_deviceHandle,
            &GetPropertiesCallback,
            reinterpret_cast<void*>(ctx),
            device->m_resourceUri.c_str(),
            nullptr,
            nullptr,
            nullptr);
    if (IPCA_OK == getStatus) {
        if ( std::cv_status::timeout == g_getPropertiesCompleteCV.wait_for(
                get_lock, std::chrono::milliseconds{ MAX_TIME_OUT_MS })) {
            status = IPCA_REQUEST_TIMEOUT;
            std::cout << "GetDeviceProperties_Impl(): time out! deviceId =" << deviceId.c_str() << std::endl;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        std::vector<CallbackCtx*>::iterator it;
        for (it = g_contexts.begin(); it != g_contexts.end(); it++) {
            if (ctx == *it) {
                std::cout << "GetDeviceProperties_Impl(): delete ctx = " << ctx << std::endl;
                delete ctx;
                g_contexts.erase(it);
                break;
            }
        }
    }
    return status;
}

IPCAStatus OCFClient::GetDeviceProperties(std::string deviceId, std::map<std::string, IPCAValueType>& properties)
{
    IPCAStatus status = IPCA_OK;

    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "GetDeviceProperties(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    if (device->m_properties.find(device->m_resourceUri) != device->m_properties.end()) {
        properties = device->m_properties[device->m_resourceUri];
        properties.erase("n");
        properties.erase("online");
        properties.erase("observers");
        std::cout << "GetDeviceProperties() : successful " << std::endl;
    }

    return status;
}

IPCAStatus OCFClient::GetDevicePropertyValue(std::string deviceId, std::string property_key, std::string& property_value)
{
    std::unique_lock<std::mutex> get_lock { m_getPropertyValueCbMutex };
    IPCAStatus status = IPCA_OK;
    CallbackCtx* ctx;

    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "GetDevicePropertyValue(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    if (device->m_online != 1 && property_key.compare("online") != 0) {
        std::cout << "GetDeviceProperties(): device is offline. don't allow to get! deviceID =" <<
            deviceId<< std::endl;
        return IPCA_FAIL;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        ctx = new CallbackCtx;
        if (!ctx) {
            std::cout << "failed to new CallbackCtx." << std::endl;
            return IPCA_FAIL;
        }

        ctx->deviceId = deviceId;
        ctx->propertyKey = property_key;
        g_contexts.push_back(ctx);
    }
    std::cout << "====GetDevicePropertyValue(): entering: ctx = " << ctx << std::endl;
    IPCAStatus getStatus = IPCAGetProperties(device->m_deviceHandle,
            &GetPropertyValueCallback,
            reinterpret_cast<void*>(ctx),
            device->m_resourceUri.c_str(),
            nullptr,
            nullptr,
            nullptr);

    if (IPCA_OK == getStatus) {
        if ( std::cv_status::timeout == g_getPropertyValueCompleteCV.wait_for(
                get_lock, std::chrono::milliseconds{ MIN_TIME_OUT_MS })) {
            status = IPCA_REQUEST_TIMEOUT;
            LOGW("GetDevicePropertyValue(): time out! deviceId: %s\n", deviceId.c_str());
            goto exit;
        }
    }

    if (!g_propertyValue.empty()) {
        std::cout << "GetDevicePropertyValue() : successful. property_key=  " << property_key.c_str()
            << ", property_value =  "<< g_propertyValue.c_str() << std::endl;
        property_value = g_propertyValue;
        g_propertyValue.clear();
    } else {
        status = IPCA_FAIL;
        std::cout << "GetDevicePropertyValue() : failed." << std::endl;
    }

exit:
    {
        std::lock_guard<std::recursive_mutex> lock(g_contextsMutex);
        std::vector<CallbackCtx*>::iterator it;
        for (it = g_contexts.begin(); it != g_contexts.end(); it++) {
            if (ctx == *it) {
                std::cout << "GetDevicePropertyValue(): delete ctx = " << ctx << std::endl;
                delete ctx;
                g_contexts.erase(it);
                break;
            }
        }
    }
    return status;
}

IPCAStatus OCFClient::NotifyDeviceProperties(
        std::string deviceId,
        IPCAPropertyBagHandle propertyBagHandle)
{
    IPCAStatus status = IPCA_OK;

    OCFDevice::Ptr device = GetDeviceById(deviceId);
    if (!device) {
        std::cout << "GetDevicePropertyValue(): can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }

    OCFDeviceCallBackHandler* pCallback = GetCallback();
    if (pCallback == nullptr) {
        std::cout << "failed to OCFClient GetCallback." << std::endl;
        return IPCA_FAIL;
    }

    std::map<std::string, IPCAValueType> properties = device->m_properties[device->m_resourceUri];
    std::map<std::string, IPCAValueType>::iterator it;

    for (it = properties.begin(); it != properties.end(); it++) {
        std::string propertyValue;
        std::string propertyKey;
        propertyKey = it->first;

        if (propertyKey.compare("n") == 0 ||
                propertyKey.compare("observers") == 0)
            continue;

        if (propertyKey.compare("online") == 0) {
            bool boolValue;
            IPCAPropertyBagGetValueBool(propertyBagHandle, propertyKey.c_str(), &boolValue);
            if (boolValue != device->m_online) {
                device->m_online = (int)boolValue;
                propertyValue = int2String(boolValue);
                if (propertyValue.compare("1") == 0)
                    pCallback->onOCFDeviceStatusChanged(deviceId, device->m_deviceType, "online");
                else if (propertyValue.compare("0") == 0)
                    pCallback->onOCFDeviceStatusChanged(deviceId, device->m_deviceType, "offline");
            }
            continue;
        }

        switch (it->second) {
            case IPCA_INTEGER:
                int intValue;
                IPCAPropertyBagGetValueInt(propertyBagHandle, propertyKey.c_str(), &intValue);
                propertyValue = int2String(intValue);
                break;
            case IPCA_BOOLEAN:
                bool boolValue;
                IPCAPropertyBagGetValueBool(propertyBagHandle, propertyKey.c_str(), &boolValue);
                propertyValue = int2String(boolValue);
                break;
            case IPCA_STRING:
                char *stringValue;
                IPCAPropertyBagGetValueString(propertyBagHandle, propertyKey.c_str(), &stringValue);
                propertyValue.assign(stringValue);
                break;
            case IPCA_DOUBLE:
                double doubleValue;
                IPCAPropertyBagGetValueDouble(propertyBagHandle, propertyKey.c_str(), &doubleValue);
                propertyValue = int2String(doubleValue);
                break;
            default:
                std::cout << "unsupported value type:" << it->second << std::endl;
                break;
        }
        device->m_propertyList[propertyKey] = propertyValue;
        pCallback->onOCFDevicePropertyChanged(deviceId, propertyKey, propertyValue);
        // FIXME: need to remove TV device when its power is off.
        if (0 == device->m_deviceType.compare(DEVICE_TYPE::LE_TV) &&
                0 == propertyKey.compare("power") &&
                0 == propertyValue.compare("0")) {
            if (device->m_observeHandle != nullptr)
                IPCACloseHandle(device->m_observeHandle, nullptr, 0);
            g_OCFDeviceList.erase(deviceId);
            pCallback->onOCFDeviceStatusChanged(deviceId, device->m_deviceType, "offline");
        } 
    }

    return status;
}

IPCAStatus OCFClient::DiscoverDevices()
{
    std::unique_lock<std::mutex> lock { m_deviceDiscoveredCbMutex };

    if (g_discoverDeviceHandle != nullptr) {
        IPCACloseHandle(g_discoverDeviceHandle, nullptr, 0);
    }

    // Start discovering devices.  
    const char* deviceTypes[] = {
        "" /* any resource type */     
    };
    const int NUMBER_OF_RESOURCE_TYPES = sizeof(deviceTypes) / sizeof(char*);

    IPCAStatus status = IPCADiscoverDevices(
            g_ipcaAppHandle,
            &DiscoverDevicesCallback,
            nullptr,
            deviceTypes,
            NUMBER_OF_RESOURCE_TYPES,
            &g_discoverDeviceHandle);

    return status;
}

IPCAStatus OCFClient::ListenDevices()
{
    OCPlatform::OCPresenceHandle presenceHandle = nullptr;
    OCStackResult result = OCPlatform::subscribePresence(presenceHandle,
            "", "",
            CT_ADAPTER_IP,
            &ListenDevicesCallback);

    return IPCA_OK;
}

// Call IPCAObserveResource();
IPCAStatus OCFClient::RequestObserve(std::string deviceId)
{
    if (g_OCFDeviceList.find(deviceId) == g_OCFDeviceList.end()) {
        std::cout << "can't find device for deviceId:" << deviceId.c_str() << std::endl;
        return IPCA_FAIL;
    }
    OCFDevice::Ptr device = g_OCFDeviceList[deviceId];

    if (device->m_observeHandle != nullptr) {
        IPCACloseHandle(device->m_observeHandle, nullptr, 0);
        device->m_observeHandle = nullptr;
    }
    return IPCAObserveResource(
                device->m_deviceHandle,
                &ResourceChangeNotificationCallback,
                reinterpret_cast<void*>(&device->m_deviceId),
                device->m_resourceUri.c_str(),
                nullptr,
                &device->m_observeHandle);
}

OCFDevice::Ptr OCFClient::GetGatewayById(std::string gatewayId)
{
    std::lock_guard<std::recursive_mutex> lock(g_globalMutex);
    std::map<std::string, OCFDevice::Ptr>::iterator it;

    for (it = g_OCFDeviceList.begin(); it != g_OCFDeviceList.end(); it++) {
        OCFDevice::Ptr device = it->second;
        if ((device->m_deviceType.compare(DEVICE_TYPE::BRIDGE) == 0) &&
                !device->m_resourceUri.empty() &&
                device->m_resourceUri.find(gatewayId) != std::string::npos) {
            std::cout << "find gateway for gatewayId: " << gatewayId.c_str() <<
                ", deviceId: "<< it->first.c_str() << std::endl;
            return device;
        }
    }

    return nullptr;
}

void OCFClient::SetCallback(OCFDeviceCallBackHandler* callback)
{
    if (callback != nullptr)
        m_callback = callback;
}

OCFDeviceCallBackHandler* OCFClient::GetCallback()
{
    return m_callback;
}


} /*OIC*/
} /*Servcie*/
} /*HB*/
