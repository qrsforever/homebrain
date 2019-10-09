/***************************************************************************
 *  LFUnitTest.cpp -
 *
 *  Created: 2019-07-11 10:17:56
 *
 *  Copyright QRS
 ****************************************************************************/

#include "LFUnitTest.h"
#include "LFTypes.h"
#include "MainHandler.h"
#include "GatewayAgentHandler.h"
#include "Log.h"

namespace HB {

static std::string gSubDeviceId("larfe.test.aircondition001");
static std::string gSubDeviceName("aircondition");
static std::string gSubProductKey("pk.2f0f787e2d5");
static std::string gSubDeviceSecret;
static std::string gSubDeviceElinkId;

int UnittestCallback::doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId)
{
    LOGD("(%s %s %s)\n", deviceId.c_str(), deviceSecret.c_str(), elinkId.c_str());
    if (gSubDeviceId != deviceId)
        abort();
    gSubDeviceSecret.assign(deviceSecret);
    gSubDeviceElinkId.assign(elinkId);
    return 0;
}

int UnittestCallback::doTopoAddReply(std::string elinkId, int code, std::string message)
{
    LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
    if (code >= 300)
       abort();
    return 0;
}

int UnittestCallback::doTopoDelReply(std::string elinkId, int code, std::string message)
{
    LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
    if (code >= 300)
       abort();
    return 0;
}

int UnittestCallback::doPropertySet(std::string elinkId, std::vector<struct DeviceProperty> &params)
{
    for (size_t i = 0; i < params.size(); ++i) {
        LOGD("%s: %s %s %s\n", elinkId.c_str(),
            params[i].name.c_str(),
            params[i].value.c_str(),
            params[i].type.c_str());
    }
    return 0;
}

int UnittestCallback::doPropertyGet(std::string elinkId, std::vector<std::string> &params)
{
    for (size_t i = 0; i < params.size(); ++i)
        LOGD("%s: %s\n", elinkId.c_str(), params[i].c_str());
    return 0;
}

int UnittestCallback::doUpgradeReply(std::string newVersion, int code, std::string message)
{
    LOGD("(%s %d %s)\n", newVersion.c_str(), code, message.c_str());
    return 0;
}

int UnittestCallback::doUpgradeCheck(std::string text)
{
    LOGD("(%s)\n", text);
    return 0;
}

bool onLFUnitTest(int arg1, int arg2)
{
    switch (arg1) {
        case UNITTEST_START:
            GAgent().setRemoteCallback(new UnittestCallback());
            mainHandler().sendMessageDelayed(
                mainHandler().obtainMessage(LFMT_HB_UNITTEST, UNITTEST_REGISTER_SUBDEV, 0), 1000);
            break;
        case UNITTEST_REGISTER_SUBDEV:
            GAgent().registerSubdev(gSubDeviceId, gSubDeviceName, gSubProductKey);
            mainHandler().sendMessageDelayed(
                mainHandler().obtainMessage(LFMT_HB_UNITTEST, UNITTEST_SUBDEV_TOPOADD, 0), 3000);
            break;
        case UNITTEST_SUBDEV_TOPOADD:
            GAgent().topoAdd(gSubDeviceElinkId, gSubDeviceSecret, gSubProductKey);
            mainHandler().sendMessageDelayed(
                mainHandler().obtainMessage(LFMT_HB_UNITTEST, UNITTEST_SUBDEV_TOPODEL, 0), 3000);
            break;
        case UNITTEST_SUBDEV_TOPODEL:
            GAgent().topoDel(gSubDeviceElinkId, gSubDeviceSecret, gSubProductKey);
            mainHandler().sendMessageDelayed(
                mainHandler().obtainMessage(LFMT_HB_UNITTEST, UNITTEST_UPGRADE_REPORT, 0), 3000);
            break;
        case UNITTEST_UPGRADE_REPORT:
            GAgent().upgradeReport("1.0", "2.0");
            mainHandler().sendMessageDelayed(
                mainHandler().obtainMessage(LFMT_HB_UNITTEST, UNITTEST_END, 0), 1000);
            break;
        case UNITTEST_END:
            break;
        default:
            ;
    }
    return true;
}

} /* namespace HB */
