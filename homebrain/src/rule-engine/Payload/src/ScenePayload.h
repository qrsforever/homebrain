/***************************************************************************
 *  ScenePayload.h -
 *
 *  Created: 2019-06-10 15:06:24
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __ScenePayload_H__
#define __ScenePayload_H__

#include "Payload.h"
#include <string>
#include <vector>

#ifdef __cplusplus

namespace HB {

typedef enum MS_ON_EVENT {
    eON_ENTER = 0,
    eON_EXIT = 1,
    eON_STORY = 2,
} ms_on_event_t;

class MicroService {
public:
    MicroService(std::string id, std::string name, std::string deps)
        : mId(id), mName(name), mDeps(deps) {}
    virtual ~MicroService() {}
    virtual std::string getConditions(std::string fmt) = 0;
    virtual std::string makeService(std::string fmt) = 0;

    std::string makeStory(std::string fmt, std::string &svr);

    std::string fullName();
protected:
    std::string mId;
    std::string mName;
    std::string mDeps;
}; /* class MicroService */

class AutoLightMS : public MicroService {
public:
    AutoLightMS(std::string id, std::string name, std::string deps)
        : MicroService(id, name, deps) {}
    ~AutoLightMS() {}

    std::string mLightUUID;
    std::string mLightStep;
    std::string mSensorUUID;
    std::string mSensorIll;

    std::string getConditions(std::string fmt);
    std::string makeService(std::string fmt);

}; /* class AutoLightMS */

class GradLightMS : public MicroService {
public:
    GradLightMS(std::string id, std::string name, std::string deps)
        : MicroService(id, name, deps) {}
    ~GradLightMS() {}

    std::string mLightUUID;
    std::string mDirection;
    std::string mStepCount;
    std::string mTimeSecondMs;

    std::string getConditions(std::string fmt);
    std::string makeService(std::string fmt);
}; /* class GradLightMS */

class SosAlarmMS : public MicroService {
public:
    SosAlarmMS(std::string id, std::string name, std::string deps)
        : MicroService(id, name, deps) {}
    ~SosAlarmMS() {}

    std::string mSosUUID;
    std::string mAlarmUUID;
    std::string mLightUUID;
    std::string mTimeSecondMs;

    std::string getConditions(std::string fmt);
    std::string makeService(std::string fmt);
}; /* class SosAlarmMS */

class DeviceControl {
public:
    DeviceControl(std::string did, std::string prop, std::string value)
        : mDid(did), mSlot(prop), mValue(value) {}
    ~DeviceControl() {};

    std::string toString(std::string fmt);

private:
    std::string mDid;
    std::string mSlot;
    std::string mValue;
}; /* class DeviceControl */

class NotifyContent {
public:
    NotifyContent (std::string id, std::string title, std::string content)
        : mID(id), mTitle(title), mContent(content) {}
    ~NotifyContent () {}

    std::string toString(std::string fmt);

private:
    std::string mID;
    std::string mTitle;
    std::string mContent;
}; /* class NotifyContent */

class ScenePayload : public Payload {
public:
    ScenePayload(std::string id, std::string name);
    ScenePayload() {};
    ~ScenePayload();

    PayloadType type() { return PT_SCENE_PAYLOAD; }

    void addAutoLightMS(ms_on_event_t event, std::string id, std::string deps, std::string lightUUID,
        std::string lightStep, std::string sensorUUID, std::string sensorIll);

    void addGradLightMS(ms_on_event_t event, std::string id, std::string deps, std::string lightUUID,
        std::string direction, std::string stepCount, std::string timeSecondMs);

    void addSosAlarmMS(ms_on_event_t event, std::string id, std::string deps, std::string mSosUUID,
        std::string mAlarmUUID, std::string mLightUUID, std::string timeSecondMs);

    void addDeviceCtrl(ms_on_event_t event, std::string did, std::string pro, std::string val);
    void addNotifyContent(ms_on_event_t event, std::string id, std::string title, std::string content);

    std::string toString();

    std::string mSid;
    std::string mName;
    std::string mRoom;
private:
    std::vector<MicroService*> mServices[3]; // onEnter(not support now) | onExit(not support now) | onStory (supported)
    std::vector<DeviceControl*> mControls[3]; // onStory(not support now)
    std::vector<NotifyContent*> mNotifies[3];

}; /* class ScenePayload */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __ScenePayload_H__ */
