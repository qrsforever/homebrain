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

#include "SysTime.h"
#include "Common.h"

#include <stdlib.h>

#if 0
#define ELINK_HOST      "10.112.39.126"
#define ELINK_ID        "el.48f8c93eb2ca9e775649128f293b66d8"
#define ELINK_USER       ELINK_ID
#define ELINK_PASSWD    "ds.3567a01ae3f3a13c0f7760e623bdbf6e"
#define ELINK_KEY       "pk.be25f066ac9"
#else
#define ELINK_HOST      "10.122.67.99"
#define ELINK_ID        "gateway"
#define ELINK_USER      "letv"
#define ELINK_PASSWD    "letv"
#define ELINK_KEY       "pk.be25f066ac9"
#endif

#define TOPIC_REGISTER  "/sys/"ELINK_KEY"/"ELINK_ID"/thing/sub/register"
#define TOPIC_TOPOADD   "/sys/"ELINK_KEY"/"ELINK_ID"/thing/topo/add"
#define TOPIC_TOPODEL   "/sys/"ELINK_KEY"/"ELINK_ID"/thing/topo/delete"

#define TOPIC_REPLY_REGISTER  "/sys/"ELINK_KEY"/"ELINK_ID"/thing/sub/registerReply"
#define TOPIC_REPLY_TOPOADD   "/sys/"ELINK_KEY"/"ELINK_ID"/thing/topo/addReply"
#define TOPIC_REPLY_TOPODEL   "/sys/"ELINK_KEY"/"ELINK_ID"/thing/topo/deleteReply"

#define TOPIC_PROPERTY_SET    "/sys/property/"ELINK_KEY"/"ELINK_ID"/set"
#define TOPIC_PROPERTY_GET    "/sys/property/"ELINK_KEY"/"ELINK_ID"/get"

#define TOPIC_REPLY_PROPERTY_SET "/sys/property/"ELINK_KEY"/"ELINK_ID"/setReply"
#define TOPIC_REPLY_PROPERTY_GET "/sys/property/"ELINK_KEY"/"ELINK_ID"/getReply"

#define TOPIC_EVENT_REPORT  "/sys/event/"ELINK_KEY"/"ELINK_ID"/upload"

using namespace rapidjson;
using namespace UTILS;

namespace HB {

static void _MessageArrived(MessageData* md)
{
    if (0 == md)
        return;
    char topicName[256] = { 0 };
    if (md->topicName->lenstring.len > 255)
        memcpy(topicName, md->topicName->lenstring.data, 255);
    else
        memcpy(topicName, md->topicName->lenstring.data, md->topicName->lenstring.len);
	MQTTMessage* message = md->message;
    // TODO
    ((char*)message->payload)[message->payloadlen] = '\0';
    GAgent().onMessage(topicName, (char*)message->payload, (int)message->payloadlen);
}

static GatewayAgent *gGA = 0;

GatewayAgent::GatewayAgent() : mThreadQuit(false), mSubCallback(0)
{
}

GatewayAgent::~GatewayAgent()
{

}

int GatewayAgent::onMessage(const char* topic, const char*payload, int length)
{
    GA_LOGD("topic = %s\n", topic);
    auto it = mTopicTypes.find(topic);
    if (it == mTopicTypes.end())
        return -1;
    switch (it->second) {
        case eRegisterReply:
            return onRegisterReply(topic, payload, length);
        case eTopoAddReply:
            return onTopoAddReply(topic, payload, length);
        case eTopoDelReply:
            return onTopoDelReply(topic, payload, length);
        case ePropertySet:
            return onPropertySet(topic, payload, length);
        case ePropertyGet:
            return onPropertyGet(topic, payload, length);
        default:
            ;
    }
    return 0;
}

int GatewayAgent::onRegisterReply(const char* topic, const char* payload, int length)
{
    if (!mSubCallback)
        return -1;

    GA_LOGI("payload = %s\n", payload);

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

    return mSubCallback->doRegisterReply(
        deviceId.GetString(),
        deviceSecret.GetString(),
        elinkId.GetString());
}

int GatewayAgent::onTopoAddReply(const char* topic, const char* payload, int length)
{
    if (!mSubCallback)
        return -1;

    GA_LOGI("payload = %s\n", payload);

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
        !result.HasMember("message")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    Value &code = result["code"];
    Value &message = result["message"];
    return mSubCallback->doTopoAddReply(
        elinkId.GetString(),
        code.GetInt(),
        message.GetString());
}

int GatewayAgent::onTopoDelReply(const char* topic, const char* payload, int length)
{
    if (!mSubCallback)
        return -1;

    GA_LOGI("payload = %s\n", payload);

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
        !result.HasMember("message")) {
        GA_LOGE("rapidjson parse result error!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    Value &code = result["code"];
    Value &message = result["message"];
    return mSubCallback->doTopoDelReply(
        elinkId.GetString(),
        code.GetInt(),
        message.GetString());
}

int GatewayAgent::onPropertySet(const char* topic, const char* payload, int length)
{
    if (!mSubCallback)
        return -1;

    GA_LOGI("payload = %s\n", payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("params") ||
        !doc.HasMember("elinkId")) {
        GA_LOGE("rapidjson parse error: not found params!\n");
        return -1;
    }
    Value &params = doc["params"];
    if (!params.IsArray()) {
        GA_LOGE("rapidjson parse error: params is not array!\n");
        return -1;
    }
    Value &elinkId = doc["elinkId"];
    std::vector<struct DeviceProperty> props;
    for (size_t i = 0; i < params.Size(); ++i) {
        Value &item = params[i];
        if (!item.IsObject())
            continue;
        for (Value::ConstMemberIterator itr = item.MemberBegin(); itr != item.MemberEnd(); ++itr) {
            struct DeviceProperty pro;
            pro.name.assign(itr->name.GetString());
            if (itr->value.IsInt()) {
                pro.type.assign("int");
                pro.value.assign(int2String(itr->value.GetInt()));
            } else if (itr->value.IsDouble()) {
                pro.type.assign("float");
                pro.value.assign(double2String(itr->value.GetDouble()));
            } else {
                pro.type.assign("string");
                pro.value.assign(itr->value.GetString());
            }
            props.push_back(pro);
        }
    }
    return mSubCallback->doPropertySet(elinkId.GetString(), props);
}

int GatewayAgent::onPropertyGet(const char* topic, const char* payload, int length)
{
    if (!mSubCallback)
        return -1;

    GA_LOGI("payload = %s\n", payload);

    Document doc;
    doc.Parse<0>(payload);
    if (doc.HasParseError()) {
        GA_LOGE("rapidjson parse error[%d]\n", doc.GetParseError());
        return -1;
    }
    if (!doc.HasMember("params") ||
        !doc.HasMember("elinkId")) {
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
    return mSubCallback->doPropertyGet(elinkId.GetString(), props);
}

int GatewayAgent::propertySetResult(std::string elinkId, int code, std::string message)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
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
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_REPLY_PROPERTY_SET, &msg);
}

int GatewayAgent::propertyGetResult(std::string elinkId, int code , std::vector<struct DeviceProperty> &props)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
    writer.Key("code");
    writer.Int(code);
    writer.Key("elinkId");
    writer.String(elinkId.c_str());
    writer.Key("result");
    writer.StartArray();
    for (size_t i = 0; i < props.size(); ++i) {
        writer.StartObject();
        writer.Key(props[i].name.c_str());
        if (props[i].type == "int") {
            writer.Int(atoi(props[i].value.c_str()));
        } else if (props[i].type == "float") {
            writer.Double(atof(props[i].value.c_str()));
        } else {
            writer.String(props[i].value.c_str());
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("propertyGetResult: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_REPLY_PROPERTY_GET, &msg);
}

int GatewayAgent::eventReport(std::vector<struct EventInfo> &events)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
    writer.Key("data");
    writer.StartArray();
    for (size_t i = 0; i < events.size(); ++i) {
        writer.StartObject();
        writer.Key(events[i].event.c_str());
        writer.String(events[i].type.c_str());
        writer.String(events[i].message.c_str());
        if (!events[i].value.empty())
            writer.String(events[i].value.c_str());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    GA_LOGD("eventReport: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_EVENT_REPORT, &msg);
}

int GatewayAgent::connectElink()
{
    GA_LOGW("init\n");

	NetworkInit(&mNetwork);
	if (NetworkConnect(&mNetwork, ELINK_HOST, 1883) < 0) {
        GA_LOGW("network connect error!\n");
        return -1;
    }
	MQTTClientInit(&mClient, &mNetwork, 3000, mSendBuf, PACKET_MAX_SIZE, mReadBuf, PACKET_MAX_SIZE);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
    data.clientID.cstring = ELINK_ID;
	data.username.cstring = ELINK_USER;
	data.password.cstring = ELINK_PASSWD;
	data.keepAliveInterval = 10;
	data.cleansession = 1;

	return MQTTConnect(&mClient, &data);
}

int GatewayAgent::registerSubdev(std::string deviceId, std::string deviceName, std::string productKey)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
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

    GA_LOGD("registerSubdev: %s\n", s.GetString());

    MQTTMessage msg;
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_REGISTER, &msg);
}

int GatewayAgent::topoAdd(std::string elinkId, std::string deviceSecret, std::string productKey)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();

    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
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
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_TOPOADD, &msg);
}

int GatewayAgent::topoDel(std::string elinkId, std::string deviceSecret, std::string productKey)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();

    writer.Key("timestamp");
    writer.Int(SysTime::GetMSecs());
    writer.Key("request_id");
    writer.String("test");
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
    msg.qos = 0;
    msg.payload = (void*)s.GetString();
    msg.payloadlen = strlen(s.GetString());
    return MQTTPublish(&mClient, TOPIC_TOPODEL, &msg);
}

void GatewayAgent::start()
{
    if (!MQTTIsConnected(&mClient))
        return;

    MQTTSubscribe(&mClient, TOPIC_REPLY_REGISTER, 0, _MessageArrived);
    MQTTSubscribe(&mClient, TOPIC_REPLY_TOPOADD, 0, _MessageArrived);
    MQTTSubscribe(&mClient, TOPIC_REPLY_TOPODEL, 0, _MessageArrived);
    MQTTSubscribe(&mClient, TOPIC_PROPERTY_SET, 0, _MessageArrived);
    MQTTSubscribe(&mClient, TOPIC_PROPERTY_GET, 0, _MessageArrived);

    mTopicTypes.insert(std::make_pair(TOPIC_REPLY_REGISTER, eRegisterReply));
    mTopicTypes.insert(std::make_pair(TOPIC_REPLY_TOPOADD, eTopoAddReply));
    mTopicTypes.insert(std::make_pair(TOPIC_REPLY_TOPODEL, eTopoDelReply));
    mTopicTypes.insert(std::make_pair(TOPIC_PROPERTY_SET, ePropertySet));
    mTopicTypes.insert(std::make_pair(TOPIC_PROPERTY_GET, ePropertyGet));
    Thread::start();
}

void GatewayAgent::run()
{
    int rc = FAILURE;
    while (!mThreadQuit) {
        rc = MQTTYield(&mClient, 1000);
        if (rc == FAILURE) {
            //TODO check network
            GA_LOGW("yield error\n");
        }
    }
    MQTTDisconnect(&mClient);
    NetworkDisconnect(&mNetwork);
}

GatewayAgent& GAgent()
{
    if (0 == gGA) {
        gGA = new GatewayAgent();
    }
    return *gGA;
}

} /* namespace HB */
