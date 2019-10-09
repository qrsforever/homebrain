/*
 * LarfeDevice.h
 *
 */

#ifndef LARFE_DEVICE_H_
#define LARFE_DEVICE_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "LarfeInternal.h"

#define MAX_OBJ 10
static const std::string light_porch_key = "pk.60726af68e8";
static const std::string light_chandelier_key = "pk.3e6b21477dd";
static const std::string light_belt_key = "pk.bd0f9fb9859";
static const std::string light_spot_key = "pk.387405b6ee8";
static const std::string door_contact_key = "pk.ee3ff25370b";
static const std::string scene_key = "pk.aabb7db5155";
static const std::string airconditioner_key = "pk.2f0f787e2d5";

typedef enum {
    DEVICE_TYPE_LIGHT_PORCH = 1,
    DEVICE_TYPE_LIGHT_CHANDELIER = 2,
    DEVICE_TYPE_LIGHT_BELT = 3,
    DEVICE_TYPE_LIGHT_SPOT = 4,
    DEVICE_TYPE_DOOR_CONTACT = 12,
    DEVICE_TYPE_SCENE = 13,
    DEVICE_TYPE_AIRCONDITIONER = 15,
    DEVICE_TYPE_GATEWAY = 99,
} DeviceType;

typedef enum {
    VAR_TYPE_STRING,
    VAR_TYPE_BOOLEAN,
    VAR_TYPE_INT,
    VAR_TYPE_UINT,
    VAR_TYPE_INT64,
    VAR_TYPE_UINT64,
    VAR_TYPE_DOUBLE,
} VarType;

typedef struct {
    std::string varName;        // attr name
    VarType type;               // attr type
    std::string varValue;       // attr value
    int index;                  // attr index. corresponding to (addr[2] % 10)
    bool writable;
    LarfeObj* obj;              // attr map to obj
} LarfeAttribute;

class LarfeDevice {
  public:
    std::string m_deviceId;
    DeviceType m_deviceType;
    std::string m_deviceName;
    std::string m_roomId;

    /* Device information from hardware */
    int m_index;
    std::vector <LarfeObj*> m_objs;
    std::vector <LarfeAttribute> m_attributes;

    LarfeDevice(std::string did, DeviceType deviceType, std::string roomId);
    virtual ~LarfeDevice();
    void dumpInfo();

    void updateAttributeList();

    LarfeObj* getAttributeObj(const std::string name);
    std::string getAttributeName(const LarfeObj* obj);

    int getAttributeValue(std::string name, std::string& value, VarType& type);
    int setAttributeValue(const std::string name, const std::string value);
};

#endif /* LARFE_DEVICE_H_ */
