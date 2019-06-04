/***************************************************************************
 *  GatewayAgent.h -
 *
 *  Created: 2019-05-20 17:29:59
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __GatewayAgent_H__
#define __GatewayAgent_H__

#include "Thread.h"
#include "mqtt/MQTTClient.h"

#include <map>
#include <vector>
#include <string>

#define PACKET_MAX_SIZE 20480
#define TOPIC_MAX_LENGTH 256

#ifdef __cplusplus

namespace HB {

enum {
    eRegisterReply,
    eTopoAddReply,
    eTopoDelReply,
    ePropertySet,
    ePropertyGet,
};

struct DeviceProperty {
    DeviceProperty(): name(""), value(""), type("") {}
    DeviceProperty(std::string n, std::string v, std::string t)
        : name(n), value(v), type(t) {}
    std::string name;
    std::string value;
    std::string type; // int, string, float
};

class SubElinkCallback {
public:
    virtual ~SubElinkCallback() {} ;

    virtual int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) = 0;
    virtual int doTopoAddReply(std::string elinkId, int code, std::string message) = 0;
    virtual int doTopoDelReply(std::string elinkId, int code, std::string message) = 0;
    virtual int doPropertySet(std::string elinkId, std::vector<struct DeviceProperty>&) = 0;
    virtual int doPropertyGet(std::string elinkId, std::vector<std::string>&) = 0;

};

struct EventInfo {
    EventInfo(): event(""), type(""), message(""), value("") {}
    EventInfo(std::string e, std::string t, std::string m, std::string v="")
        : event(e), type(t), message(m), value(v) {}
    std::string event;
    std::string type;
    std::string message;
    std::string value;
};

class GatewayAgent : public ::UTILS::Thread {
public:
    GatewayAgent();
    ~GatewayAgent();

    void setSubCallback(SubElinkCallback *cb) { mSubCallback = cb; }

    int registerSubdev(std::string deviceId, std::string deviceName, std::string productKey);
    int topoAdd(std::string elinkId, std::string deviceSecret, std::string productKey);
    int topoDel(std::string elinkId, std::string deviceSecret, std::string productKey);

    int propertySetResult(std::string elinkId, int code, std::string message);
    int propertyGetResult(std::string elinkId, int code, std::vector<struct DeviceProperty>&);
    int eventReport(std::vector<struct EventInfo> &events);

    int connectElink();
    void start();
    void run();

    int onMessage(const char*, const char*, int);
private:
    int onRegisterReply(const char*, const char*, int);
    int onTopoAddReply(const char*, const char*, int);
    int onTopoDelReply(const char*, const char*, int);
    int onPropertySet(const char*, const char*, int);
    int onPropertyGet(const char*, const char*, int);

private:
    MQTTClient mClient;
    Network mNetwork;
    bool mThreadQuit;

    unsigned char mReadBuf[PACKET_MAX_SIZE];
    unsigned char mSendBuf[PACKET_MAX_SIZE];

    SubElinkCallback *mSubCallback;

    std::map<std::string, int> mTopicTypes;

}; /* class GatewayAgent */

GatewayAgent& GAgent();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __GatewayAgent_H__ */
