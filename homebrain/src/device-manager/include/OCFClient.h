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
#ifndef DEVICE_MANAGER_OCFCLIENT_H_
#define DEVICE_MANAGER_OCFCLIENT_H_

#include "iotivity_config.h"
#include <string>
#include <list>
#include <vector>
#include "ipca.h"
#include "OCPlatform.h"
#include "OCApi.h"
#include "OCFDevice.h"

using namespace OC;
/**
 * This file contains the declaration of classes and its members related to OCFClient.
 */

namespace OIC {
namespace Service {
namespace HB {

//using namespace std;
/**
 * This class contains a set of functions to describe a OCF client
 * 
 */

class OCFDeviceCallBackHandler {
public:
		virtual ~OCFDeviceCallBackHandler() {} ;
		virtual void onOCFDeviceStatusChanged(const std::string deviceId, const std::string deviceType, std::string status) = 0;
		virtual void onOCFDevicePropertyChanged(const std::string deviceId, std::string propertyKey, std::string propertyValue) = 0;
};

// Callback from IPCAObserveResource().
void IPCA_CALL ResourceChangeNotificationCallback(IPCAStatus result, void* context, IPCAPropertyBagHandle propertyBagHandle); 
// Callback when device is discovered.
void IPCA_CALL DiscoverDevicesCallback(void* context, IPCADeviceStatus deviceStatus, const IPCADiscoveredDeviceInfo* deviceInfo);
// IPCASetProperties() completion callback
void IPCA_CALL SetPropertiesCallback(IPCAStatus result, void* context, IPCAPropertyBagHandle propertyBagHandle);
// IPCAGetProperties() completion callback.
void IPCA_CALL GetPropertiesCallback(IPCAStatus result, void* context, IPCAPropertyBagHandle propertyBagHandle);
void IPCA_CALL GetPropertyValueCallback(IPCAStatus result, void* context, IPCAPropertyBagHandle propertyBagHandle);
void ListenDevicesCallback(OCStackResult result, const unsigned int nonce, const std::string& hostAddress);

OCFDevice::Ptr GetDeviceById(std::string deviceId);

typedef struct {
    std::string deviceId;
    std::string propertyKey;
} CallbackCtx;

class OCFClient {
public:
    typedef std::shared_ptr<OCFClient> Ptr;
    static OCFClient* GetInstance();

    virtual ~OCFClient();

    // init/deinit pair
    IPCAStatus Init();
    IPCAStatus Deinit();

    // functions to handle device properties. must be invoked after init().
    IPCAStatus SetDeviceProperty(std::string deviceId, std::string propertyKey, std::string propertyValue, bool async);
    IPCAStatus SetDeviceProperties(std::string deviceId, std::map<std::string, std::string>& propertyList, bool async);
    IPCAStatus GetDeviceProperties(std::string deviceId, std::map<std::string, IPCAValueType>& properties);
    IPCAStatus GetDeviceProperties_Impl(std::string deviceId);
    IPCAStatus GetDevicePropertyValue(std::string deviceId, std::string propertyKey, std::string& propertyValue);
    IPCAStatus NotifyDeviceProperties(std::string deviceId, IPCAPropertyBagHandle propertyBagHandle);

    // request discovering devices.
    IPCAStatus DiscoverDevices();
    // request listen devices online/offline.
    IPCAStatus ListenDevices();
    // request properties observe for specific device.
    IPCAStatus RequestObserve(std::string deviceId);

    // Get gateway by gateway id
    OCFDevice::Ptr GetGatewayById(std::string gatewayId);

    // Delete device
    IPCAStatus DeleteDevice(std::string deviceId);

    // set/get device owned status
    IPCAStatus GetDeviceOwnedStatus(std::string deviceId, OCFDeviceOwnedStatus& ownedStatus);
    IPCAStatus SetDeviceOwnedStatus(std::string deviceId, OCFDeviceOwnedStatus ownedStatus);

    // set/get callback handle, SetCallback should be invoked before init().
    void SetCallback(OCFDeviceCallBackHandler* callback);
    OCFDeviceCallBackHandler* GetCallback();

private:
    OCFClient();
    class Garbage {
        public:
            ~Garbage() {
                if (OCFClient::m_instance != nullptr)
                    delete OCFClient::m_instance;
            }
    };
private:
    std::mutex m_deviceDiscoveredCbMutex;
    // for setprop
    std::mutex m_setPropertiesCbMutex;
    // for getprop
    std::mutex m_getPropertiesCbMutex;
    std::mutex m_getPropertyValueCbMutex;
    IPCAHandle m_discoverDeviceHandle;

    OCFDeviceCallBackHandler* m_callback;
    static OCFClient* m_instance;
    static Garbage m_garbage;
    bool m_initialized;
};

} /*OIC*/
} /*Servcie*/
} /*HB*/
#endif /* DEVICE_MANAGER_OCFCLIENT_H_ */
