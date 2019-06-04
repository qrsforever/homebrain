/***************************************************************************
 *  GatewayAgentHandler.cpp -
 *
 *  Created: 2019-05-21 18:25:36
 *
 *  Copyright QRS
 ****************************************************************************/

#include "GatewayAgentHandler.h"
#include "GatewayAgentLog.h"
#include "GatewayAgent.h"

using namespace UTILS;

namespace HB {

static GatewayAgentHandler *gGAHandler = 0;

void GatewayAgentHandler::handleMessage(Message *msg)
{
    GA_LOGD("msg(%d, %d, %d)\n", msg->what, msg->arg1, msg->arg2);
    switch (msg->what) {
        case GA_CONNECT_AGAIN:
        case GA_NETWORK_OK:
            if (GAgent().connectElink() < 0)
                sendEmptyMessageDelayed(GA_CONNECT_AGAIN, 3000);
            else {
                GAgent().start();
#if UNITTEST
                sendMessage(obtainMessage(GA_UNITTEST, 0, 0));
#endif
            }
            break;
#if UNITTEST
        case GA_UNITTEST:
            doUnitTest(msg);
            break;
#endif
    }
    return;
}

#if UNITTEST

class UnittestSubElinkCallback : public SubElinkCallback {
public:
    virtual ~UnittestSubElinkCallback() {} ;

    int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) {
        GA_LOGD("(%s %s %s)\n", deviceId.c_str(), deviceSecret.c_str(), elinkId.c_str());
        return 0;
    }
    int doTopoAddReply(std::string elinkId, int code, std::string message) {
        GA_LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
        return 0;
    }
    int doTopoDelReply(std::string elinkId, int code, std::string message) {
        GA_LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
        return 0;
    }
    int doPropertySet(std::string elinkId, std::vector<struct DeviceProperty> &params) {
        for (size_t i = 0; i < params.size(); ++i) {
            GA_LOGD("%s: %s %s %s\n", elinkId.c_str(),
                params[i].name.c_str(),
                params[i].value.c_str(),
                params[i].type.c_str());
        }
        return 0;
    }
    int doPropertyGet(std::string elinkId, std::vector<std::string> &params) {
        for (size_t i = 0; i < params.size(); ++i)
            GA_LOGD("%s: %s\n", elinkId.c_str(), params[i].c_str());
        return 0;
    }
};

void GatewayAgentHandler::doUnitTest(::UTILS::Message *msg)
{
    switch (msg->arg1) {
        case 0:
            GAgent().setSubCallback(new UnittestSubElinkCallback());
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 1, 0), 1000);
            break;
        case 1:
            GAgent().registerSubdev("deviceId1", "deviceName1", "productKey1");
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 2, 0), 1000);
            break;
        case 2:
            GAgent().topoAdd("elinkId1", "deviceScret1", "productKey1");
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 3, 0), 1000);
            break;
        case 3:
            GAgent().topoDel("elinkId1", "deviceScret1", "productKey1");
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 4, 0), 1000);
            break;
        case 4:
            GAgent().propertySetResult("elinkId1", 200, "Set Property Ok");
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 5, 0), 1000);
            break;
        case 5: {
            std::vector<struct DeviceProperty> props;
            struct DeviceProperty prop1("Power", "1", "int");
            struct DeviceProperty prop2("Level", "1", "string");
            props.push_back(prop1);
            props.push_back(prop2);
            GAgent().propertyGetResult("elinkId1", 200, props);
            sendMessageDelayed(obtainMessage(GA_UNITTEST, 6, 0), 1000);
            } break;
        case 6: {
            std::vector<struct EventInfo> events;
            struct EventInfo event1("SmokeCheckAlarm", "alarm", "fire your home");
            struct EventInfo event2("ImmersionCheckAlarm", "alarm", "water your home");
            events.push_back(event1);
            events.push_back(event2);
            GAgent().eventReport(events);
            } break;
        default:
            break;
    }
}
#endif

GatewayAgentHandler& GAHandler()
{
    if (0 == gGAHandler) {
        gGAHandler = new GatewayAgentHandler();
    }
    return *gGAHandler;
}

} /* namespace HB */
