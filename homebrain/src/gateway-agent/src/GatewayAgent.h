/**
 * @file GatewayAgent.h
 * @brief gateway agent talk with elink cloud
 * @author QRS
 * @version 1.0
 * @date 2019-05-20
 */

#ifndef __GatewayAgent_H__
#define __GatewayAgent_H__

#include "mqtt/MQTTClient.h"

#include <map>
#include <vector>
#include <string>
#include <utility>

#define PACKET_MAX_SIZE 20480
#define TOPIC_MAX_LENGTH 2560

#ifdef __cplusplus

namespace HB {

/**
 * @brief abstract pure virtual class, need extend and implement all methods
 */
class RemoteEventCallback {
public:
    virtual ~RemoteEventCallback() {} ;

    /* sub device */
    /**
     * @brief doRegisterReply  remote callback of register subdevice @more GatewayAgent::registerSubdev
     *
     * @param deviceId  the device id
     * @param deviceSecret  the device secret
     * @param elinkId  the elink id
     *
     * @return 0 if no error
     */
    virtual int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) = 0;

    /**
     * @brief doTopoAddReply  remote callback of add sub device topo @more GatewayAgent::topoAdd
     *
     * @param elinkId  the elink id
     * @param code  the error code
     * @param message  the more information
     *
     * @return 0 if no error
     */
    virtual int doTopoAddReply(std::string elinkId, int code, std::string message) = 0;

    /**
     * @brief doTopoDelReply  remote callback of add sub device topo @more GatewayAgent::topoAdd
     *
     * @param elinkId  the elink id
     * @param code  the error code
     * @param message  the more information
     *
     * @return 0 if no error
     */
    virtual int doTopoDelReply(std::string elinkId, int code, std::string message) = 0;

    /* device property */
    /**
     * @brief doPropertySet  remote control, set property of one device
     *
     * @param rid  the request id
     * @param elinkId  the elink Id
     * @param props  std::vector, properties list
     *
     * @return 0 if no error
     */
    virtual int doPropertySet(std::string rid, std::string elinkId, std::vector<std::pair<std::string, std::string>>&props) = 0;

    /**
     * @brief doPropertySet  remote control, get property of one device
     *
     * @param rid  the request id
     * @param elinkId  the elink Id
     * @param props  std::vector, properties list
     *
     * @return 0 if no error
     */
    virtual int doPropertyGet(std::string rid, std::string elinkId, std::vector<std::string>&props) = 0;

    /* ota */
    /**
     * @brief doUpgradeReply  notify the system upgrade
     *
     * @param newVersion  the version be upgrading.
     * @param code  the erorr code
     * @param message  the message about upgrade
     *
     * @return 0 if no error
     */
    virtual int doUpgradeReply(std::string newVersion, int code, std::string message) = 0;

    /**
     * @brief doUpgradeCheck  notify the system request and report the latest version if exist
     *
     * @param text maybe null
     *
     * @return 0 if no error
     */
    virtual int doUpgradeCheck(std::string text) = 0;

    /* log */
    /**
     * @brief doLogReport  notify the system upload the log file to log server.
     *
     * @param elinkId  the elink id
     * @param key  the log key, maybe the user account
     * @param logId  the log id, any string
     *
     * @return 0 if no error
     */
    virtual int doLogReport(std::string elinkId, std::string key, std::string logId) = 0;

    /* system cmd */
    /**
     * @brief doSystemCmd  remote control to exec some tasks(reset, reboot...)
     *
     * @param cmd  the remote command
     * @param params  the arguments of the command needed.
     *
     * @return  0 if no error
     */
    virtual int doSystemCmd(std::string cmd, std::string params) = 0;
};

struct EventInfo {
    EventInfo():event(""), type(""), message(""), value("") {}
    EventInfo(std::string e, std::string t, std::string m, std::string v="")
        :event(e), type(t), message(m), value(v) {}
    std::string event;
    std::string type; // alarm, status, error
    std::string message;
    std::string value;
};

/** @brief GatewayAgent class */
class GatewayAgent {
public:
    GatewayAgent();
    ~GatewayAgent();

    /**
     * @brief init  initialize GatewayAgent instance
     *
     * @pre must call the method of setRemoteCallback first
     *
     * @param host  the mqtt host to connect
     * @param clientId  the id to use to this client
     * @param user  a username to be used for authenticating with the broker
     * @param password  a password to be used for authenticating with the broker
     * @param key  the product key of the gateway host
     *
     * @return -1 if error else 0
     */
    int init(const std::string &host, const std::string &clientId,
        const std::string &user, const std::string &password, const std::string &key);

    /**
     * @brief connect connect the elink
     *
     * @note if return -1, need try connect again.
     *
     * @return  -1 if error else 0
     */
    int connect();

    /**
     * @brief yield cycle check and handle mqtt message from elink
     *
     * @param ms the max micro seconds wait the mqtt message
     *
     * @return -1 if tcp connect error else 0
     */
    int yield(int ms);

    /**
     * @brief setRemoteCallback  set the remote callback instance before call init
     *
     * @param cb the instance of RemoteEventCallback class
     *
     * @see HB::RemoteEventCallback
     */
    void setRemoteCallback(RemoteEventCallback *cb) { mEventCallback = cb; }

    /**
     * @brief registerSubdev  publish message, register the sub device
     *
     * @param deviceId  the unique id of the sub device
     * @param deviceName  the name of the sub device
     * @param productKey  the product key that generated from elink but pre-set on device or host
     *
     * @return -1 if error else 0
     */
    int registerSubdev(std::string deviceId, std::string deviceName, std::string productKey);

    /**
     * @brief topoAdd  publish message, add the device in a topo which maybe one user account
     *
     * @param elinkId  the elink id generated from response the @see registerSubdev
     * @param deviceSecret  the device secret from response the @see registerSubdev
     * @param productKey  the product key that generated from elink but pre-set on device or host
     *
     * @return -1 if error else 0
     */
    int topoAdd(std::string elinkId, std::string deviceSecret, std::string productKey);

    /**
     * @brief topoDel  publish message, del the device in a topo which maybe one user account
     *
     * @param elinkId  the elink id generated from response the @see registerSubdev
     * @param deviceSecret  the device secret from response the @see registerSubdev
     * @param productKey  the product key that generated from elink but pre-set on device or host
     *
     * @return -1 if error else 0
     */
    int topoDel(std::string elinkId, std::string deviceSecret, std::string productKey);

    /**
     * @brief propertySetResult return the result of @see RemoteEventCallback::doPropertySet
     *
     * @param rid  the request id in the request of @see RemoteEventCallback::doPropertySet
     * @param elinkId the elink id
     * @param code  the result code
     * @param message the result message, maybe null
     *
     * @return -1 if error else 0
     */
    int propertySetResult(std::string rid, std::string elinkId, int code, std::string message);

    /**
     * @brief propertyGetResult return the result of @see RemoteEventCallback::doPropertyGet
     *
     * @param rid  the request id in the request of @see RemoteEventCallback::doPropertySGt
     * @param elinkId the elink id
     * @param code  the result code
     * @param props  vector, the result list contains the pair of <name, value>
     *
     * @return -1 if error else 0
     */
    int propertyGetResult(std::string rid, std::string elinkId, int code, std::vector<std::pair<std::string, std::string>>& props);

    /**
     * @brief propertyGetResult return the result of @see RemoteEventCallback::doPropertyGet
     *
     * @param rid  the request id in the request of @see RemoteEventCallback::doPropertyGet
     * @param elinkId the elink id
     * @param code  the result code
     * @param name  the name of the property
     * @param value  the value of the property
     *
     * @return -1 if error else 0
     */
    int propertyGetResult(std::string rid, std::string elinkId, int code, std::string name, std::string value);

    /**
     * @brief eventReport  report event to elink
     *
     * @param eid  the event id
     * @param events vector, the event list contains @see EventInfo
     *
     * @note if the event type is "status", EventInfo::event is the property name, EventInfo::value is the property value
     *
     * @return -1 if error else 0
     */
    int eventReport(std::string eid, std::vector<struct EventInfo> &events);

    /**
     * @brief eventReport  report event to elink
     *
     * @param eid  the event id
     * @param event  then event name
     * @param message  then event message, if type is "status", this can be null
     * @param type  the event type
     * @param value  if type is "status", value is the property value of the event
     *
     * @return -1 if error else 0
     */
    int eventReport(std::string eid, std::string event, std::string message, std::string type, std::string value);

    /**
     * @brief upgradeReport  report current software version and exists the latest version
     *
     * @param curVersion  the current version
     * @param newVersion  the latest version on the server.
     *
     * @return -f if error else 0
     */
    int upgradeReport(std::string curVersion, std::string newVersion);

    const std::string& getUsername() { return mUsername; }
    const std::string& getClientId() { return mClientId; }
    const std::string& getKey() { return mKey; }

    int onMessage(const char*, const char*, int);
private:
    int onRegisterReply(const char*, const char*, int);
    int onTopoAddReply(const char*, const char*, int);
    int onTopoDelReply(const char*, const char*, int);
    int onPropertySet(const char*, const char*, int);
    int onPropertyGet(const char*, const char*, int);
    int onUpgradeReply(const char*, const char*, int);
    int onUpgradeCheck(const char*, const char*, int);
    int onLogReport(const char*, const char*, int);
    int onSystemCmd(const char*, const char*, int);

private:
    MQTTClient mClient;
    Network mNetwork;

    unsigned char mReadBuf[PACKET_MAX_SIZE];
    unsigned char mSendBuf[PACKET_MAX_SIZE];

    RemoteEventCallback *mEventCallback;

    std::map<std::string, int> mSubTopics;

    std::string mHost;
    std::string mClientId;
    std::string mUsername;
    std::string mPassword;
    std::string mKey;

    std::map<int, std::string> mTopics;
}; /* class GatewayAgent */

GatewayAgent& GAgent();

} /* namespace HB */


int EIOT_Init(HB::RemoteEventCallback *cb, const std::string &host, const std::string &clientId, const std::string &user, const std::string &password, const std::string &key);
int EIOT_Connect();
int EIOT_Yield(int ms);
int EIOT_RegisterSubdev(std::string deviceId, std::string deviceName, std::string productKey);
int EIOT_TopoAdd(std::string elinkId, std::string deviceSecret, std::string productKey);
int EIOT_TopoDel(std::string elinkId, std::string deviceSecret, std::string productKey);
int EIOT_PropertySetReply(std::string rid, std::string elinkId, int code, std::string message);
int EIOT_PropertyGetReply(std::string rid, std::string elinkId, int code, std::vector<std::pair<std::string, std::string>>&);
int EIOT_PropertyGetReply(std::string rid, std::string elinkId, int code, std::string name, std::string value);
int EIOT_EventReport(std::string eid, std::string event, std::string message, std::string type, std::string value);
int EIOT_UpgradeReport(std::string curVersion, std::string newVersion);

#endif /* __cplusplus */

#endif /* __GatewayAgent_H__ */
