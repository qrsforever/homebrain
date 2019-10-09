#ifndef _INTERFACECONTROL_H_
#define _INTERFACECONTROL_H_
#include <string>
#include <list>
#include <map>
#include <string>
#include "iotSock.h"
#include <vector>
#include "HBDevice.h"
#include "HBDeviceManager.h"
using namespace std;




class Interface_Control
{
public:
    Interface_Control();
    virtual ~Interface_Control();

    //HBDeviceCallBackHandler  interfaceCallback();

    void setupIdMatchAddr();
    void setDevStatusToHB();
    void getDevStatusToHB();

public :
    HBDeviceManager *devManager;
    std::vector<HBDevice::Ptr> lightDevVector;
    std::vector<HBDevice::Ptr> curtainDevVector;
    std::vector<HBPropertyInfoPtr> PropertiesVector;

    U8 countLight;
    U8 countCurtain;
    map<string, UINT16> idAddrLightMap;
    map<string, UINT16> idAddrCurtainMap;
};

class Interface_Callback:public HBDeviceCallBackHandler
{
public:
    Interface_Callback();
    virtual ~Interface_Callback();

    virtual void onDeviceStatusChanged(const std::string deviceId, const HBDeviceType deviceType, HBDeviceStatus status);
    virtual void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value);

    Interface_Control *ifaceControl_callback;
    Iot_485 *iot485_callback;
};
#endif
