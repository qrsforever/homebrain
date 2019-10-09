/*
 * LarfeDevice.cpp
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "LarfeDevice.h"
#include "LarfeClientLog.h"
using namespace std;

LarfeDevice::LarfeDevice(
        std::string did,
        DeviceType deviceType,
        std::string roomId)
    :m_deviceId(did),
    m_deviceType(deviceType),
    m_roomId(roomId),
    m_index(0)
{
    if (m_deviceType == DEVICE_TYPE_LIGHT_PORCH) {
        LarfeAttribute attr = {"LightSwitch", VAR_TYPE_BOOLEAN, "", 1, true, NULL};
        m_attributes.push_back(attr);
        m_deviceName.assign("玄关灯");
    } else if (m_deviceType == DEVICE_TYPE_LIGHT_CHANDELIER) {
        LarfeAttribute attr = {"LightSwitch", VAR_TYPE_BOOLEAN, "", 1, true, NULL};
        m_attributes.push_back(attr);
        m_deviceName.assign("餐厅吊灯");
    } else if (m_deviceType == DEVICE_TYPE_LIGHT_BELT) {
        LarfeAttribute attr = {"LightSwitch", VAR_TYPE_BOOLEAN, "", 1, true, NULL};
        m_attributes.push_back(attr);
        m_deviceName.assign("客餐厅灯带");
    } else if (m_deviceType == DEVICE_TYPE_LIGHT_SPOT) {
        LarfeAttribute attr = {"LightSwitch", VAR_TYPE_BOOLEAN, "", 1, true, NULL};
        m_attributes.push_back(attr);
        m_deviceName.assign("客餐厅射灯");
    } else if (m_deviceType == DEVICE_TYPE_SCENE) {
        LarfeAttribute attr = {"SceneMode", VAR_TYPE_INT, "", 1, true, NULL};
        m_attributes.push_back(attr);
        m_deviceName.assign("场景");
    } else if (m_deviceType == DEVICE_TYPE_AIRCONDITIONER) {
        LarfeAttribute attr1 = {"WorkMode", VAR_TYPE_INT, "", 2, true, NULL};
        m_attributes.push_back(attr1);
        LarfeAttribute attr2 = {"WindSpeed", VAR_TYPE_INT, "", 4, true, NULL};
        m_attributes.push_back(attr2);
        LarfeAttribute attr3 = {"PowerSwitch", VAR_TYPE_BOOLEAN, "", 1, true, NULL};
        m_attributes.push_back(attr3);
        LarfeAttribute attr4 = {"TargetTemperature", VAR_TYPE_DOUBLE, "", 3, true, NULL};
        m_attributes.push_back(attr4);
        LarfeAttribute attr5 = {"CurrentTemperature", VAR_TYPE_DOUBLE, "", 5, false, NULL};
        m_attributes.push_back(attr5);
        m_deviceName.assign("空调");
    } else if (m_deviceType == DEVICE_TYPE_DOOR_CONTACT) {
        LarfeAttribute attr1 = {"ContactState", VAR_TYPE_BOOLEAN, "", 1, false, NULL};
        m_attributes.push_back(attr1);
        LarfeAttribute attr2 = {"ProtectionSwitch", VAR_TYPE_BOOLEAN, "", 2, true, NULL};
        m_attributes.push_back(attr2);
        LarfeAttribute attr3 = {"TamperAlarm", VAR_TYPE_BOOLEAN, "", 3, false, NULL};
        m_attributes.push_back(attr3);
        m_deviceName.assign("门磁");
    }
    LC_LOGI("deviceId: %s, deviceType: %d, deviceName: %s\n", m_deviceId.c_str(), m_deviceType, m_deviceName.c_str());
}

LarfeDevice::~LarfeDevice()
{
    LC_LOGI("device is deleted!");
    dumpInfo();
}

void LarfeDevice::updateAttributeList()
{
    //update attr's obj
    vector<LarfeAttribute>::iterator it;
    for (it = m_attributes.begin(); it != m_attributes.end(); it++) {
        for (int i = 0; i < m_objs.size(); i++) {
            int index = m_objs[i]->m_addrValueStatus[0] ?
                (m_objs[i]->m_addrValueStatus[2] % 10) : (m_objs[i]->m_addrValueSet[2] % 10);
            if (it->index == index) {
                it->obj = m_objs[i];
                it->varValue = m_objs[i]->m_value;
                break;
            }
        }
    }
}

LarfeObj* LarfeDevice::getAttributeObj(const std::string name)
{
    vector<LarfeAttribute>::iterator it;
    for (it = m_attributes.begin(); it != m_attributes.end(); it++) {
        if (it->varName == name)
            return it->obj;
    }

    LC_LOGE("cannot find attribute! name: %s\n", name.c_str());
    return NULL;
}

std::string LarfeDevice::getAttributeName(const LarfeObj* obj)
{
    vector<LarfeAttribute>::iterator it;
    for (it = m_attributes.begin(); it != m_attributes.end(); it++) {
        if (it->obj == obj) {
            return it->varName;
        }
    }

    LC_LOGE("cannot find attribute! obj: %p\n", obj);
    return "";
}

int LarfeDevice::getAttributeValue(std::string name, std::string& value, VarType& type)
{
    vector<LarfeAttribute>::iterator it;
    for (it = m_attributes.begin(); it != m_attributes.end(); it++) {
        if (it->varName == name) {
            value = it->varValue;
            type = it->type;
            LC_LOGD("success. name: %s, value: %s\n", name.c_str(), value.c_str());
            return 0;
        }
    }

    LC_LOGE("failed. cannot find attribute! name: %s\n", name.c_str());
    return -1;
}

int LarfeDevice::setAttributeValue(const std::string name, const std::string value)
{
    vector<LarfeAttribute>::iterator it;
    for (it = m_attributes.begin(); it != m_attributes.end(); it++) {
        if (it->varName == name) {
            it->varValue = value;
            LC_LOGD("success. name: %s, value: %s\n", name.c_str(), value.c_str());
            return 0;
        }
    }

    LC_LOGE("failed. cannot find attribute! name: %s\n", name.c_str());
    return -1;
}

void LarfeDevice::dumpInfo()
{
}

