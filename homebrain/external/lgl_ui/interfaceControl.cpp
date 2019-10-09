#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#include "common.h"
#include "iotSock.h"
#include "interfaceControl.h"

UINT16 bufLight [24]  =  {0x100,0x101,0x102,0x110,0x111,0x112,0x113,0x120,
                          0x121,0x122,0x123,0x130,0x131,0x140,0x141,0x150,
                          0x151,0x152,0x160,0x161,0x162,0x170,0x171,0x172};  //保存lgl 灯的地址

UINT16 bufCurtain[24] = {0x200,0x201,0x202,0x210,0x211,0x212,0x213,0x220,
                         0x221,0x222,0x223,0x230,0x231,0x240,0x241,0x250,
                         0x251,0x252,0x260,0x261,0x262,0x270,0x271,0x272};  //保存了lgl 的窗帘的地址

Interface_Callback::Interface_Callback()
{
    IOT_COMM_INFO("Interface_Callback construct\n");
}

Interface_Callback::~Interface_Callback()
{
    IOT_COMM_INFO("~Interface_Callback deconstruct\n");
}

typedef enum {
    HB_DEVICE_TYPE_LIGHT = 0,
    HB_DEVICE_TYPE_ALARM,
    HB_DEVICE_TYPE_DOORCONTACT,
    HB_DEVICE_TYPE_ENVSENSOR,
    HB_DEVICE_TYPE_CURTAIN,
    HB_DEVICE_TYPE_PLUG,
} HBDeviceType;

void Interface_Callback::onDeviceStatusChanged(const std::string deviceId, const HBDeviceType deviceType, HBDeviceStatus status)
{
    //TODO check deviceID 是否在idAddrLightMap...  中，如果不在则进行更新idAddrLightMap...  及配置文件(末尾增加)
    U8 numberSize = 0;

    if(deviceType == HB_DEVICE_TYPE_LIGHT)
    {
        numberSize = ifaceControl_callback->idAddrLightMap.size();
        ifaceControl_callback->countLight = numberSize+1;
        ifaceControl_callback->idAddrLightMap.insert(pair<string, UINT16>(devId, bufLight[numberSize]));   //ID+ADDR  LIGHT
    }
    IOT_COMM_INFO("Interface_Control onDeviceStatusChanged\n");
}

void Interface_Callback::onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value)
{
    if(propertyKey == "switch")
    {
        U8 lightBuf[8]= {0xaa,0x20,0x3,0x0,0x60,0x1,0x2e};

        lightBuf[5] = (U8)atoi(value);
        map<string, UINT16> ::iterator Iter;
        Iter = ifaceControl_callback->idAddrLightMap.find(deviceId);
        lightBuf[3] = Iter->second&0xff00>>8;
        lightBuf[4] = Iter->first&0x00ff;

        iot485_callback->Iot_485_conLight(lightBuf);
    }

    if(propertyKey == "curtain")
    {
        U8 curtainBuf[8]= {0xaa,0x20,0x3,0x0,0x60,0x1,0x2e};

        curtainBuf[5] = (U8)atoi(value);
        map<string, UINT16> ::iterator Iter;
        Iter = ifaceControl_callback->idAddrCurtainMap.find(deviceId);
        curtainBuf[3] = Iter->second&0xff00>>8;
        curtainBuf[4] = Iter->first&0x00ff;

        iot485_callback->Iot_485_conLight(curtainBuf);
    }

    IOT_COMM_INFO("Interface_Control onDevicePropertyChanged\n");
}

Interface_Control::Interface_Control()
{
    devManager = new HBDeviceManager();
    countLight = 0;
    countCurtain = 0;
    IOT_COMM_INFO("Interface_Control construct\n");
}

Interface_Control::~Interface_Control()
{
    delete devManager;
    devManager = NULL;
    IOT_COMM_INFO("~Interface_Control deconstruct\n");
}

string Interface_Control::getDevIdByAddr(UINT16 devAdd, Iot_DevType devType)
{
    IOT_COMM_INFO("Enter getDevIdByAddr\n");
    map<string, UINT16> ::iterator iter;

    if(devType == DEVICE_TYPE_LIGHT)
    {
        for(iter = idAddrLightMap.begin(); iter != idAddrLightMap.end(); iter++){
            IOT_COMM_INFO("idAddrLightMap::iter->first id=%s, iter->second addr=0x%x\n", iter->first, iter->second);
            if(devAdd == iter->second)
            {
                return iter->first;
            }
        }
    }

    if(devType == DEVICE_TYPE_CURTAIN)
    {
        for(iter = idAddrCurtainMap.begin(); iter != idAddrCurtainMap.end(); iter++){
            IOT_COMM_INFO("idAddrCurtainMap::iter->first id=%s, iter->second addr=0x%x\n", iter->first, iter->second);
            if(devAdd == iter->second)
            {
                return iter->first;
            }
        }
    }

    return NULL;
}

void Interface_Control::setupIdMatchAddr()
{
    //TODO 每次更新idAddrLightMap... 则需要从配置文件开始，如果lightDevVector 有新的devid 则更新idAddrLightMap...  及配置文件
    //TODO 先从配置文件更新idAddrLightMap...

    std::string devId;
    vector<HBDevice::Ptr>::iterator it;

    countLight = 0;
    lightDevVector = devManager->getDevicesByType(HB_DEVICE_TYPE_LIGHT);
    for(it=lightDevVector.begin(); it!=lightDevVector.end(); it++)
    {
        devId = it->getDeviceId();

        //TODO 查询配置文件表，是否存在device id ,如果有则从配置文件中insert
        idAddrLightMap.insert(pair<string, UINT16>(devId, bufLight[countLight]));   //ID+ADDR  LIGHT

        IOT_COMM_INFO("setupIdMatchAddr devid=%s, countLight=%d\n", devId, countLight);
        countLight++;
    }

    curtainDevVector = devManager->getDevicesByType(HB_DEVICE_TYPE_CURTAIN);
    countCurtain = 0;
    for(it=curtainDevVector.begin(); it!=curtainDevVector.end(); it++)
    {
        devId = it->getDeviceId();
        //TODO 查询配置文件表，是否存在device id ,如果有则从配置文件中insert
        idAddrCurtainMap.insert(pair<string, UINT16>(devId, bufCurtain[countCurtain]));   //ID+ADDR  CURTAIN

        IOT_COMM_INFO("setupIdMatchAddr devid=%s, countCurtain=%d\n", devId, countCurtain);
        countCurtain++;
    }

    //TODO 更新表，更新到配置文件，以后只能查询不能变更
    IOT_COMM_INFO("Interface_Control setupIdMatchAddr\n");
}

void Interface_Control::setDevStatusToHB(UINT16 devAdd, Iot_DevType devType, U8 *buf)
{
    string devId;
    vector<HBPropertyInfoPtr> ptr;
    string categoryKey;

    if(devType == DEVICE_TYPE_LIGHT)
    {
         //get device ID
        U8 devValueL[2];
        sprintf(devValueL, "%d", buf[5]);//按正常位数转换

        devId = getDevIdByAddr(devAdd, devType);
        ptr = devManager->getDeviceProperties(devId);
        categoryKey = ptr->key;

        devManager->setDevicePropertyValue(devId, categoryKey, devValueL);
    }

    if(devType == DEVICE_TYPE_CURTAIN)
    {
         //get device ID
        U8 devValueC[2];
        sprintf(devValueC, "%d", buf[5]);//按正常位数转换

        devId = getDevIdByAddr(devAdd, devType);
        ptr = devManager->getDeviceProperties(devId);
        categoryKey = ptr->key;

        devManager->setDevicePropertyValue(devId, categoryKey, devValueC, false);
    }

    IOT_COMM_INFO("Interface_Control setDevStatusToHB\n");
}

void Interface_Control::getDevStatusToHB(UINT16 devAdd, Iot_DevType devType, U8 *bufChar)
{
    //TODO 基本不需要底层返回get ， 在底层状态发生
    string devId;
    vector<HBPropertyInfoPtr> ptr;
    string categoryKey;

    if(devType == DEVICE_TYPE_LIGHT)
    {
        string sDevValueL;
        devId = getDevIdByAddr(devAdd, devType);
        ptr = devManager->getDeviceProperties(devId);
        categoryKey = ptr->key;

        devManager->getDevicePropertyValue(devId, categoryKey, &sDevValueL);
        *bufChar = (char)atoi(sDevValueL.data());
    }

    if(devType == DEVICE_TYPE_CURTAIN)
    {
        string sDevValueC;
        devId = getDevIdByAddr(devAdd, devType);
        ptr = devManager->getDeviceProperties(devId);
        categoryKey = ptr->key;

        devManager->getDevicePropertyValue(devId, categoryKey, &sDevValueC, false);
        *bufChar = (char)atoi(sDevValueC.data());
    }

    IOT_COMM_INFO("Interface_Control getDevStatusToHB\n");
}

