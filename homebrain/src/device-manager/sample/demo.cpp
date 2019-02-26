#include <unistd.h>
#include <iostream>
#include <string>
#include "Common.h"

#include "Log.h"
#include "HBDeviceManager.h"
using namespace OIC::Service::HB;
using namespace UTILS;


class testCallBack: public HBDeviceCallBackHandler {
public:
		~testCallBack();

		void onDeviceStatusChanged(const std::string deviceId, const std::string deviceType, HBDeviceStatus status);
		void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value);
};

void testCallBack::onDeviceStatusChanged(const std::string deviceId, const std::string deviceType, HBDeviceStatus status) 
{

}

void testCallBack::onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value)
{
            std::cout << "onDevicePropertyChanged: device id=" << deviceId.c_str() 
                << ". propertyKey="<< propertyKey.c_str()
                << ". propertyValue=" << value.c_str() << std::endl;
}

testCallBack::~testCallBack()
{
    std::cout << "~testCallback()" << std::endl;
}

int main(int argc, char* argv[])
{
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);

    testCallBack* pCallback = new testCallBack();
    HBDeviceManager *manager = HBDeviceManager::GetInstance();
    manager->SetCallback(pCallback);
    manager->Init();
    
    std::map<std::string, OCFDevice::Ptr> deviceList;
    manager->GetAllDevices(deviceList);

    std::map<std::string, std::string> devices;
    manager->GetDeviceList(devices);
    std::map<std::string, std::string>::iterator it;
    for (it = devices.begin(); it != devices.end(); it++) {
        std::cout << "GetDeviceList(): deviceId: " << (it->first).c_str() <<
            ", deviceType: " << (it->second).c_str() << std::endl;
    }



    std::map<std::string, OCFDevice::Ptr>::iterator device;
    std::map<std::string, HBPropertyType> propertyList;
    std::map<std::string, std::string> properties;
    std::string propertyKey;
    std::string propertyValue;
    for (device = deviceList.begin(); device != deviceList.end(); device++) {
        std::string deviceId = device->first;
        propertyList.clear();
        if (0 != manager->GetDevicePropertyList(deviceId, propertyList))
            manager->GetDevicePropertyList(deviceId, propertyList);
        //sleep(1);
        std::map<std::string, HBPropertyType>::iterator property;
        for (property = propertyList.begin(); property != propertyList.end(); property++) {
            propertyKey = property->first;
            manager->GetDevicePropertyValue(deviceId, propertyKey, propertyValue, false);
            //manager->SetDevicePropertyValue(deviceId, propertyKey, propertyValue, false);
        }
        manager->GetDevicePropertyListWithValue(deviceId, properties);
        std::map<std::string, std::string>::iterator it;
        for (it = properties.begin(); it != properties.end(); it++) {
            std::cout << "GetDevicePropertyListWithValue(): propertyKey: " << it->first <<
                ", propertyValue: " << it->second << std::endl;
        }
        manager->SetDevicePropertyListWithValue(deviceId, properties, true);
    }
    sleep(600);
    int i = 0;
    int j = 0, k= 0;
    /*while (0 != manager->GetDevicePropertyList("00a17a88-a01a-02a1-4af9-acf0f0d0d0ea", propertyList)) {
            ;
    }*/
    while (0) {
#if 0
        i = (i + 1) % 100;
        j = (j + 50) % 500;
        std::cout << "before SetDevicePropertyValue:  " << i << std::endl;
        manager->SetDevicePropertyValue("00a17a88-a01a-02a1-4af9", "saturation", int2String(i).c_str(), false);
        std::cout << "after SetDevicePropertyValue " <<std::endl;
        manager->GetDevicePropertyValue("00a17a88-a01a-02a1-4af9", "saturation", propertyValue, false);
        std::cout << "loop count " << k++ <<std::endl;
#endif
        //sleep(0.5);
    }
    manager->Deinit();
    delete manager;
    delete pCallback;

    return 0;
}
