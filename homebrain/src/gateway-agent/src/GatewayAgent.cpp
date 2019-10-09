/***************************************************************************
 *  GatewayAgent.cpp -
 *
 *  Created: 2019-05-20 17:30:16
 *
 *  Copyright QRS
 ****************************************************************************/

#include "GatewayAgent.h"
#include "GatewayAgentLog.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <unistd.h>
#include <stdlib.h>

using namespace rapidjson;

std::string _GenRandom(unsigned int a = 100000, unsigned int b = 999999)
{
    srand((unsigned)time(0));
    char id[32] = { 0 };
    sprintf(id, "%u", (rand()%(b-a+1))+a);
    return std::string(id);
}

#define SHOW_TOPIC 1

namespace HB {

/**
 * @brief elink topics (pub/sub) codes
 */
enum ElinkTopicCode {
    /// numeric 0 device register (pub)
    TOPIC_REGISTER = 0,
    /// numeric 1 device add topo (pub)
    TOPIC_TOPOADD,
    /// numeric 2 subdevice remove topo (pub)
    TOPIC_TOPODEL,
    /// numeric 3 the register result reply from elink (sub)
    TOPIC_REPLY_REGISTER,
    /// numeric 4 the add topo result reply from elink (sub)
    TOPIC_REPLY_TOPOADD,
    /// numeric 5 the remove topo result reply from elink (sub)
    TOPIC_REPLY_TOPODEL,
    /// numeric 6 elink set property(ies) of one device (sub)
    TOPIC_PROPERTY_SET,
    /// numeric 7 elink get property(ies) of one device (sub)
    TOPIC_PROPERTY_GET,
    /// numeric 8 the result reply of 'set property' (pub)
    TOPIC_REPLY_PROPERTY_SET,
    /// numeric 9 the result reply of 'get property' (pub)
    TOPIC_REPLY_PROPERTY_GET,
    /// numeric 10 report event to elink (pub)
    TOPIC_EVENT_REPORT,
    /// numeric 11 report gateway host local version and remote server version to elink (pub)
    TOPIC_UPGRADE_REPORT,
    /// numeric 12 do upgrade system (sub)
    TOPIC_REPLY_UPGRADE,
    /// numeric 13 do check system version (sub)
    TOPIC_CHECK_UPGRADE,
    /// numeric 14 do report log (sub)
    TOPIC_LOG_REPORT,
    /// numeric 15 do system commands (sub)
    TOPIC_SYSTEM_CMD,
};

static void _MessageArrived(MessageData* md)
{/*{{{*/
    if (0 == md)
        return;
    char topicName[256] = { 0 };
    if (md->topicName->lenstring.len > 255) {
        memcpy(topicName, md->topicName->lenstring.data, 255);
    } else {
        memcpy(topicName, md->topicName->lenstring.data, md->topicName->lenstring.len);
    }
	MQTTMessage* message = md->message;
    // TODO
    ((char*)message->payload)[message->payloadlen] = '\0';
    GAgent().onMessage(topicName, (char*)message->payload, (int)message->payloadlen);
}/*}}}*/

static GatewayAgent *gGA = 0;

GatewayAgent::GatewayAgent() : mEventCallback(0)
{
}

GatewayAgent::~GatewayAgent()
{

}

int GatewayAgent::init(const std::string &host, const std::string &clientId,
        const std::string &user, const std::string &password, const std::string &key)
{/*{{{*/
    GA_LOGD("init(%s %s %s %s)\n", host.c_str(), clientId.c_str(), user.c_str(), key.c_str());

    if (!mEventCallback) {
        GA_LOGE("not set event callback\n");
        return -1;
    }

    mHost.assign(host);
    mClientId.assign(clientId);
    mUsername.assign(user);
    mPassword.assign(password);
    mKey.assign(key);

    mTopics[TOPIC_REGISTER] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/sub/register";
    mTopics[TOPIC_TOPOADD] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/topo/add";
    mTopics[TOPIC_TOPODEL] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/topo/delete";
    mTopics[TOPIC_REPLY_REGISTER] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/sub/registerReply";
    mTopics[TOPIC_REPLY_TOPOADD] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/topo/addReply";
    mTopics[TOPIC_REPLY_TOPODEL] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/topo/deleteReply";
    mTopics[TOPIC_PROPERTY_SET] = std::string("/sys/property/") + mKey + "/" + mClientId + "/set";
    mTopics[TOPIC_PROPERTY_GET] = std::string("/sys/property/") + mKey + "/" + mClientId + "/get";
    mTopics[TOPIC_REPLY_PROPERTY_SET] = std::string("/sys/property/") + mKey + "/" + mClientId + "/setReply";
    mTopics[TOPIC_REPLY_PROPERTY_GET] = std::string("/sys/property/") + mKey + "/" + mClientId + "/getReply";
    mTopics[TOPIC_EVENT_REPORT] = std::string("/sys/event/") + mKey + "/" + mClientId + "/upload";
    mTopics[TOPIC_UPGRADE_REPORT] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/ota/upgrade";
    mTopics[TOPIC_REPLY_UPGRADE] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/ota/upgradeReply";
    mTopics[TOPIC_CHECK_UPGRADE] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/ota/checkUpgrade";
    mTopics[TOPIC_LOG_REPORT] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/log/report";
    mTopics[TOPIC_SYSTEM_CMD] = std::string("/sys/") + mKey + "/" + mClientId + "/thing/sys/cmd";

#if SHOW_TOPIC
    std::map<int, std::string>::iterator it;
    for (it = mTopics.begin(); it != mTopics.end(); ++it)
        GA_LOGD("pub and sub topic: [%s]\n", it->second.c_str());
#endif
    return 0;
}/*}}}*/

int GatewayAgent::connect()
{/*{{{*/
    GA_LOGD("gateway agent connect\n");
    if (MQTTIsConnected(&mClient)) {
        MQTTDisconnect(&mClient);
        NetworkDisconnect(&mNetwork);
    }
    NetworkInit(&mNetwork);
    if (NetworkConnect(&mNetwork, (char*)mHost.c_str(), 1883) < 0) {
        GA_LOGW("Network Connect Error!\n");
        return -1;
    }
    MQTTClientInit(&mClient, &mNetwork, 3000, mSendBuf, PACKET_MAX_SIZE, mReadBuf, PACKET_MAX_SIZE);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)mClientId.c_str();
    data.username.cstring = (char*)mUsername.c_str();
    data.password.cstring = (char*)mPassword.c_str();
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    GA_LOGD("host[%s:%d] clientID[%s] username[%s] password[%s]\n", mHost.c_str(),
        1883, mClientId.c_str(), mUsername.c_str(), mPassword.c_str());

    if (MQTTConnect(&mClient, &data) < 0 || !MQTTIsConnected(&mClient)) {
        GA_LOGE("Elink Mqtt Connect Error!\n");
        return -1;
    }

    GA_LOGD("Elink Mqtt Connect Ok!\n");

    mSubTopics.insert(std::make_pair(mTopics[TOPIC_REPLY_REGISTER].c_str(), (int)TOPIC_REPLY_REGISTER));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_REPLY_TOPOADD].c_str(), (int)TOPIC_REPLY_TOPOADD));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_REPLY_TOPODEL].c_str(), (int)TOPIC_REPLY_TOPODEL));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_PROPERTY_SET].c_str(), (int)TOPIC_PROPERTY_SET));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_PROPERTY_GET].c_str(), (int)TOPIC_PROPERTY_GET));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_REPLY_UPGRADE].c_str(), (int)TOPIC_REPLY_UPGRADE));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_CHECK_UPGRADE].c_str(), (int)TOPIC_CHECK_UPGRADE));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_LOG_REPORT].c_str(), (int)TOPIC_LOG_REPORT));
    mSubTopics.insert(std::make_pair(mTopics[TOPIC_SYSTEM_CMD].c_str(), (int)TOPIC_SYSTEM_CMD));

    std::map<std::string, int>::iterator it;
    for (it = mSubTopics.begin(); it != mSubTopics.end(); ++it) {
        if (0 != MQTTSubscribe(&mClient, it->first.c_str(), (QoS)0, _MessageArrived)) {
            GA_LOGE("MQTTSubscribe[%s] error!\n", it->first.c_str());
            return -1;
        }
        GA_LOGD("sub topic: [%s]\n", it->first.c_str());
    }
    return 0;
}/*}}}*/

int GatewayAgent::yield(int ms)
{/*{{{*/
    if (!MQTTIsConnected(&mClient))
        return -1;
    if (FAILURE == MQTTYield(&mClient, ms))
        return -1;
    return 0;
}/*}}}*/

int GatewayAgent::onMessage(const char* topic, const char*payload, int length)
{/*{{{*/
    GA_LOGD("topic = %s\n", topic);
    std::map<std::string, int>::iterator it = mSubTopics.find(topic);
    if (it == mSubTopics.end())
        return -1;
    switch (it->second) {
        case TOPIC_REPLY_REGISTER:
            return onRegisterReply(topic, payload, length);
        case TOPIC_REPLY_TOPOADD:
            return onTopoAddReply(topic, payload, length);
        case TOPIC_REPLY_TOPODEL:
            return onTopoDelReply(topic, payload, length);
        case TOPIC_PROPERTY_SET:
            return onPropertySet(topic, payload, length);
        case TOPIC_PROPERTY_GET:
            return onPropertyGet(topic, payload, length);
        case TOPIC_REPLY_UPGRADE:
            return onUpgradeReply(topic, payload, length);
        case TOPIC_CHECK_UPGRADE:
            return onUpgradeCheck(topic, payload, length);
        case TOPIC_LOG_REPORT:
            return onLogReport(topic, payload, length);
        case TOPIC_SYSTEM_CMD:
            return onSystemCmd(topic, payload, length);
        default:
            ;
    }
    return 0;
}/*}}}*/

int GatewayAgent::onRegisterReply(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("result")) {
        GA_LOGE("rapidjson parse error: not found reuslt!\n");
        return -1;
    }
    Value &result = doc["result"];
    if (!result.HasMember("deviceId") ||
        !result.HasMember("deviceSecret") ||
        !result.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    Value &deviceId = result["deviceId"];
    Value &deviceSecret = result["deviceSecret"];
    Value &elinkId = result["elinkId"];

    return mEventCallback->doRegisterReply(
        deviceId.GetString(),
        deviceSecret.GetString(),
        elinkId.GetString());
}/*}}}*/

int GatewayAgent::onTopoAddReply(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("result") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found reuslt!\n");
        return -1;
    }
    Value &result = doc["result"];
    if (!result.IsArray() || result.Size() == 0) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    for (size_t i = 0; i < result.Size(); ++i) {
        Value &item = result[i];
        Value &elinkId = item["elinkId"];
        Value &code = item["code"];
        Value &message = item["message"];
        mEventCallback->doTopoAddReply(
            elinkId.GetString(), code.GetInt(), message.GetString());
    }
    return 0;
}/*}}}*/

int GatewayAgent::onTopoDelReply(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("result") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found reuslt!\n");
        return -1;
    }
    Value &result = doc["result"];
    if (!result.HasMember("code")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    Value &code = result["code"];
    return mEventCallback->doTopoDelReply(
        elinkId.GetString(),
        code.GetInt(), "");
}/*}}}*/

int GatewayAgent::onPropertySet(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("params") ||
        !doc.HasMember("elinkId") ||
        !doc.HasMember("requestId")) {
        GA_LOGE("rapidjson parse error: not found params!\n");
        return -1;
    }
    Value &params = doc["params"];
    if (!params.IsArray()) {
        GA_LOGE("rapidjson parse error: params is not array!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    std::vector<std::pair<std::string, std::string>> props;
    for (size_t i = 0; i < params.Size(); ++i) {
        Value &item = params[i];
        if (!item.IsObject())
            continue;
        for (Value::ConstMemberIterator itr = item.MemberBegin(); itr != item.MemberEnd(); ++itr)
            props.push_back(std::make_pair(itr->name.GetString(), itr->value.GetString()));
    }
    return mEventCallback->doPropertySet(doc["requestId"].GetString(), elinkId.GetString(), props);
}/*}}}*/

int GatewayAgent::onPropertyGet(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("params") ||
        !doc.HasMember("elinkId") ||
        !doc.HasMember("requestId")) {
        GA_LOGE("rapidjson parse error: not found params!\n");
        return -1;
    }
    Value &params = doc["params"];
    if (!params.IsArray()) {
        GA_LOGE("rapidjson parse error: params is not array!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    std::vector<std::string> props;
    for (size_t i = 0; i < params.Size(); ++i)
        props.push_back(params[i].GetString());
    return mEventCallback->doPropertyGet(doc["requestId"].GetString(), elinkId.GetString(), props);
}/*}}}*/

int GatewayAgent::onUpgradeReply(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("result") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found reuslt!\n");
        return -1;
    }
    Value &result = doc["result"];
    if (!result.HasMember("code") ||
        !result.HasMember("newVersion")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    Value &newv = result["newVersion"];
    Value &code = result["code"];
    return mEventCallback->doUpgradeReply(
        newv.GetString(), code.GetInt(), "");
}/*}}}*/

int GatewayAgent::onUpgradeCheck(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    return mEventCallback->doUpgradeCheck(payload);
}/*}}}*/

int GatewayAgent::onLogReport(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("logId") ||
        !doc.HasMember("productKey") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found logId!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    Value &logId = doc["logId"];
    Value &key = doc["productKey"];

    return mEventCallback->doLogReport(elinkId.GetString(), key.GetString(), logId.GetString());
}/*}}}*/

int GatewayAgent::onSystemCmd(const char* topic, const char* payload, int length)
{/*{{{*/
    GA_LOGI("%s: payload = %*s\n", topic, length, payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("result") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found reuslt!\n");
        return -1;
    }
    Value &result = doc["result"];
    if (!result.HasMember("cmd")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    std::string cmd = result["cmd"].GetString();
    std::string params("");
    if (result.HasMember("cmdPara")) {
        // TODO json object
        params = "";
    }
    return mEventCallback->doSystemCmd(cmd, params);
}/*}}}*/

int GatewayAgent::propertySetResult(std::string rid, std::string elinkId, int code, std::string message)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(rid.c_str());
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("result");
    writer.StartObject();
    writer.Key("code");
    writer.Int(code);
    writer.Key("message");
    writer.String(message.c_str());
    writer.EndObject();
    writer.EndObject();

    GA_LOGD("propertySetResult: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_REPLY_PROPERTY_SET].c_str(), &msg);
}/*}}}*/

int GatewayAgent::propertyGetResult(std::string rid, std::string elinkId, int code, std::string name, std::string value)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(rid.c_str());
    writer.Key("code");
    writer.Int(code);
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("result");
    writer.StartArray();

    writer.StartObject();
    writer.Key(name.c_str());
    writer.String(value.c_str());
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("propertyGetResult: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_REPLY_PROPERTY_GET].c_str(), &msg);
}/*}}}*/

int GatewayAgent::propertyGetResult(std::string rid, std::string elinkId, int code, std::vector<std::pair<std::string, std::string>>&props)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(rid.c_str());
    writer.Key("code");
    writer.Int(code);
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("result");
    writer.StartArray();
    for (size_t i = 0; i < props.size(); ++i) {
        writer.StartObject();
        writer.Key(props[i].first.c_str());
        writer.String(props[i].second.c_str());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("propertyGetResult: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_REPLY_PROPERTY_GET].c_str(), &msg);
}/*}}}*/

int GatewayAgent::eventReport(std::string eid, std::vector<struct EventInfo> &events)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("elinkId");
    if (eid.empty())
        writer.String(mClientId.c_str());
    else
        writer.String(eid.c_str());
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("data");
    writer.StartArray();
    for (size_t i = 0; i < events.size(); ++i) {
        writer.StartObject();
        writer.Key("event");
        writer.String(events[i].event.c_str());
        if (!events[i].message.empty()) {
            writer.Key("message");
            writer.String(events[i].message.c_str());
        }
        writer.Key("type");
        writer.String(events[i].type.c_str());
        if (!events[i].value.empty()) {
            writer.Key("value");
            writer.String(events[i].value.c_str());
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("eventReport: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_EVENT_REPORT].c_str(), &msg);
}/*}}}*/

int GatewayAgent::eventReport(std::string eid, std::string event, std::string message, std::string type, std::string value)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("elinkId");
    writer.String(eid.c_str());
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("data");
    writer.StartArray();
    writer.StartObject();
    writer.Key("event");
    writer.String(event.c_str());
    if (!message.empty()) {
        writer.Key("message");
        writer.String(message.c_str());
    }
    writer.Key("type");
    writer.String(type.c_str());
    if (!value.empty()) {
        writer.Key("value");
        writer.String(value.c_str());
    }
    writer.EndObject();
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("eventReport: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_EVENT_REPORT].c_str(), &msg);
}/*}}}*/

int GatewayAgent::registerSubdev(std::string deviceId, std::string deviceName, std::string productKey)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("elinkId");
    writer.String(mClientId.c_str());
    writer.Key("params");
    writer.StartObject();
    writer.Key("deviceId");
    writer.String(deviceId.c_str());
    writer.Key("deviceName");
    writer.String(deviceName.c_str());
    writer.Key("productKey");
    writer.String(productKey.c_str());
    writer.EndObject();
    writer.EndObject();

    GA_LOGD("registerSubdev: %s: %s\n", mTopics[TOPIC_REGISTER].c_str(), s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_REGISTER].c_str(), &msg);
}/*}}}*/

int GatewayAgent::topoAdd(std::string elinkId, std::string deviceSecret, std::string productKey)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();

    writer.Key("elinkId");
    writer.String(mClientId.c_str());
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("params");

    writer.StartArray();
    writer.StartObject();
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("deviceSecret");
    writer.String(deviceSecret.c_str());
    writer.Key("productKey");
    writer.String(productKey.c_str());
    writer.EndObject();
    writer.EndArray();

    writer.EndObject();

    GA_LOGD("topoAdd: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_TOPOADD].c_str(), &msg);
}/*}}}*/

int GatewayAgent::topoDel(std::string elinkId, std::string deviceSecret, std::string productKey)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();

    writer.Key("elinkId");
    writer.String(mClientId.c_str());
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("params");

    writer.StartArray();
    writer.StartObject();
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("deviceSecret");
    writer.String(deviceSecret.c_str());
    writer.Key("productKey");
    writer.String(productKey.c_str());
    writer.EndObject();
    writer.EndArray();

    writer.EndObject();

    GA_LOGD("topoDel: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_TOPODEL].c_str(), &msg);
}/*}}}*/

int GatewayAgent::upgradeReport(std::string curVersion, std::string newVersion)
{/*{{{*/
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();

    writer.Key("elinkId");
    writer.String(mClientId.c_str());
    writer.Key("timestamp");
    writer.Int64(GetMSecs());
    writer.Key("requestId");
    writer.String(_GenRandom().c_str());
    writer.Key("params");

    writer.StartObject();
    writer.Key("currentVersion");
    writer.String(curVersion.c_str());
    writer.Key("newVersion");
    writer.String(newVersion.c_str());
    writer.EndObject();

    writer.EndObject();

    GA_LOGD("upgrade: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = (QoS)0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, mTopics[TOPIC_UPGRADE_REPORT].c_str(), &msg);
}/*}}}*/

GatewayAgent& GAgent()
{
    if (0 == gGA)
        gGA = new GatewayAgent();
    return *gGA;
}
} /* namespace HB */

int EIOT_Init(HB::RemoteEventCallback *cb, const std::string &host, const std::string &clientId, const std::string &user, const std::string &password, const std::string &key)
{
    HB::GAgent().setRemoteCallback(cb);
    return HB::GAgent().init(host, clientId, user, password, key);
}

int EIOT_Connect()
{
    return HB::GAgent().connect();
}

int EIOT_Yield(int ms)
{
    return HB::GAgent().yield(ms);
}

int EIOT_RegisterSubdev(std::string deviceId, std::string deviceName, std::string productKey)
{
    return HB::GAgent().registerSubdev(deviceId, deviceName, productKey);
}

int EIOT_TopoAdd(std::string elinkId, std::string deviceSecret, std::string productKey)
{
    return HB::GAgent().topoAdd(elinkId, deviceSecret, productKey);
}

int EIOT_TopoDel(std::string elinkId, std::string deviceSecret, std::string productKey)
{
    return HB::GAgent().topoDel(elinkId, deviceSecret, productKey);
}

int EIOT_PropertySetReply(std::string rid, std::string elinkId, int code, std::string message)
{
    return HB::GAgent().propertySetResult(rid, elinkId, code, message);
}

int EIOT_PropertyGetReply(std::string rid, std::string elinkId, int code, std::vector<std::pair<std::string, std::string>>&props)
{
    return HB::GAgent().propertyGetResult(rid, elinkId, code, props);
}

int EIOT_PropertyGetReply(std::string rid, std::string elinkId, int code, std::string name, std::string value)
{
    return HB::GAgent().propertyGetResult(rid, elinkId, code, name, value);
}

int EIOT_EventReport(std::string eid, std::string event, std::string message, std::string type, std::string value)
{
    return HB::GAgent().eventReport(eid, event, message, type, value);
}

int EIOT_UpgradeReport(std::string curVersion, std::string newVersion)
{
    return HB::GAgent().upgradeReport(curVersion, newVersion);
}

