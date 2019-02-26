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

#include <string>
#include <iostream>
#include "CommonApi.h"
#include "HBDeviceManagerLog.h"
#include "OCFDevice.h"

namespace OIC {
namespace Service {
namespace HB {

OCFDevice::OCFDevice(IPCAAppHandle appHandle, std::string id, std::string interId) :
    m_deviceId(id),
    m_deviceHandle(nullptr),
    m_observeHandle(nullptr),
    m_ownedStatus(UNBINDED), m_online(-1),
    m_deviceIdInter(interId),
    m_ipcaAppHandle(appHandle),
    m_deviceInfo(nullptr),
    m_platformInfo(nullptr)
{
    m_resourceUri.clear();
    m_properties.clear();
}

OCFDevice::~OCFDevice()
{
    if (m_deviceHandle != nullptr)
    {
        IPCACloseDevice(m_deviceHandle);
    }

    if (m_deviceInfo != nullptr)
    {
        IPCAFreeDeviceInfo(m_deviceInfo);
    }

    if (m_platformInfo != nullptr)
    {
        IPCAFreePlatformInfo(m_platformInfo);
    }
    m_resourceUri.clear();
    m_properties.clear();
}

IPCAStatus OCFDevice::OpenDevice()
{
    if (m_deviceHandle != nullptr)
    {
        return IPCA_OK;
    }
    else
    {
        return IPCAOpenDevice(m_ipcaAppHandle, m_deviceIdInter.c_str(), &m_deviceHandle);
    }
}

IPCADeviceInfo* OCFDevice::GetDeviceInfo()
{
    IPCAStatus status;

    if (m_deviceInfo != nullptr)
    {
        return m_deviceInfo;
    }

    if (OpenDevice() != IPCA_OK)
    {
        return nullptr;
    }

    status = IPCAGetDeviceInfo(m_deviceHandle, &m_deviceInfo);

    if (IPCA_OK != status)
    {
        return nullptr;
    }

    return m_deviceInfo;
}

IPCAPlatformInfo* OCFDevice::GetPlatformInfo()
{
    IPCAStatus status;

    if (m_platformInfo != nullptr)
    {
        return m_platformInfo;
    }

    if (OpenDevice() != IPCA_OK)
    {
        return nullptr;
    }

    status = IPCAGetPlatformInfo(m_deviceHandle, &m_platformInfo);

    if (IPCA_OK != status)
    {
        return nullptr;
    }

    return m_platformInfo;
}

ResourceList OCFDevice::GetResourceInfo()
{
    IPCAStatus status;

    ResourceList emptyResourceList;
    if (OpenDevice() != IPCA_OK)
    {
        return emptyResourceList;
    }

    char** resourcePathList;
    size_t resourceListCount;
    status = IPCAGetResources(m_deviceHandle,
                nullptr, nullptr, &resourcePathList, &resourceListCount);
    if (IPCA_OK != status)
    {
        return emptyResourceList;
    }

    m_resourceList.clear();
    for (size_t i = 0 ; i < resourceListCount ; i++)
    {
        char** resourceTypes;
        size_t resourceTypeCount;
        status = IPCAGetResourceTypes(m_deviceHandle,
                    resourcePathList[i], &resourceTypes, &resourceTypeCount);
        if (IPCA_OK == status)
        {
            for (size_t j = 0 ; j < resourceTypeCount; j++)
            {
                m_resourceList[resourcePathList[i]].push_back(resourceTypes[j]);
                DM_LOGD("device resourceUri: %s, resourceType %s\n", resourcePathList[i], resourceTypes[j]);
            }
            IPCAFreeStringArray(resourceTypes, resourceTypeCount);
        }
        else
        {
            std::cout << "Failed IPCAGetResourceTypes() for resource: " << resourcePathList[i];
            std::cout << std::endl;
        }
    }

    if (m_resourceUri.empty()) {
        std::vector<std::string> resourceTypeList;
        for (auto const& resource : m_resourceList) {
            for (auto const& resourceType : resource.second) {
                if (0 == DEVICE_TYPE::BRIDGE.compare(resourceType)) {
                    DM_LOGD("a Gateway device is found.\n");
                    m_deviceType = DEVICE_TYPE::BRIDGE;
                    resourceTypeList.push_back(RESOURCE_TYPE::GATEWAY);
                    break;
                } else if (0 == DEVICE_TYPE::KK_SOSALARM.compare(resourceType)) {
                    DM_LOGD("a SOS Alarm device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_SOSALARM;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::KK_DOORCONTACT.compare(resourceType)) {
                    DM_LOGD("a Door Contact device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_DOORCONTACT;
                    resourceTypeList.push_back(RESOURCE_TYPE::CONTACT_STATUS);
                    break;
                } else if (0 == DEVICE_TYPE::KK_SCENECTRL.compare(resourceType)) {
                    DM_LOGD("a Scence Board device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_SCENECTRL;
                    resourceTypeList.push_back(RESOURCE_TYPE::SCENE_STATUS);
                    break;
                } else if (0 == DEVICE_TYPE::KK_LIGHTCTRL.compare(resourceType)) {
                    DM_LOGD("a 3-lights Board device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_LIGHTCTRL;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::KK_SMART_PLUG.compare(resourceType)) {
                    DM_LOGD("a Smart Plug device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_SMART_PLUG;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::KK_ENVSENSOR.compare(resourceType)) {
                    DM_LOGD("a Env Sensor device is found.\n");
                    m_deviceType = DEVICE_TYPE::KK_ENVSENSOR;
                    resourceTypeList.push_back(RESOURCE_TYPE::TEMPERATURE);
                    resourceTypeList.push_back(RESOURCE_TYPE::HUMIDITY);
                    resourceTypeList.push_back(RESOURCE_TYPE::ILLUMINANCE);
                    break;
                } else if (0 == DEVICE_TYPE::HUE_BRI_LIGHT.compare(resourceType)) {
                    DM_LOGD("a Hue Light device is found.\n");
                    m_deviceType = DEVICE_TYPE::HUE_BRI_LIGHT;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::HUE_COLOR_LIGHT.compare(resourceType)) {
                    DM_LOGD("a Hue Light device is found.\n");
                    m_deviceType = DEVICE_TYPE::HUE_COLOR_LIGHT;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::HUE_CT_LIGHT.compare(resourceType)) {
                    DM_LOGD("a Hue Light device is found.\n");
                    m_deviceType = DEVICE_TYPE::HUE_CT_LIGHT;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::HUE_COLOR_CT_LIGHT.compare(resourceType)) {
                    DM_LOGD("a Hue Light device is found.\n");
                    m_deviceType = DEVICE_TYPE::HUE_COLOR_CT_LIGHT;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == DEVICE_TYPE::LE_TV.compare(resourceType)) {
                    DM_LOGD("a Le TV device is found.\n");
                    m_deviceType = DEVICE_TYPE::LE_TV;
                    resourceTypeList.push_back(RESOURCE_TYPE::BINARYSWITCH);
                    break;
                } else if (0 == resourceType.compare("x.org.iotivity.sample.elevator")) {
                    m_deviceType = "oic.d.elevator";
                    resourceTypeList.push_back("x.org.iotivity.sample.elevator");
                    break;
                }
            }
            if (!m_deviceType.empty())
                break;
        }

        //FIXME: only compare the first of resource types. 
        if (!resourceTypeList.empty()) {
            for (auto const& resource : m_resourceList) {
                for (auto const& resourceType : resource.second) {
                    if (0 == resourceTypeList[0].compare(resourceType)) {
                        m_resourceUri = resource.first;
                        DM_LOGD("device m_resourceUri: %s\n", m_resourceUri.c_str());
                        break;
                    }
                }
                if (!m_resourceUri.empty())
                    break;
            }
        }
    }
    IPCAFreeStringArray(resourcePathList, resourceListCount);
    return m_resourceList;
}

} /*OIC*/
} /*Servcie*/
} /*HB*/

