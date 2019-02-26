/***************************************************************************
 *  DeviceDataChannel.cpp - Device Data Channel Impl
 *
 *  Created: 2018-06-14 15:05:46
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DeviceDataChannel.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "InstancePayload.h"
#include "MainPublicHandler.h"
#include "MessageTypes.h"
#include "Common.h"
#include "Log.h"
#include <map>

using namespace UTILS;

namespace HB {

DeviceDataChannel::DeviceDataChannel()
    : mH(ruleHandler())
{
}

DeviceDataChannel::~DeviceDataChannel()
{

}

int DeviceDataChannel::init()
{
    return 0;
}

void DeviceDataChannel::statusChanged(const std::string &deviceId, const std::string &deviceName, int status)
{
    LOGT("(%s, %s, %d)\n", deviceId.c_str(), deviceName.c_str(), status);
    switch (status) {
        case 2: /* HB_DEVICE_STATUS_ONLINE: */
            {
                std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
                payload->mInsName = innerOfInsname(deviceId);
                payload->mClsName = deviceName;
                mH.sendMessage(mH.obtainMessage(RET_INSTANCE_ADD, payload));
            }
            break;
        case 3: /* HB_DEVICE_STATUS_OFFLINE: */
            {
                std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
                payload->mInsName = innerOfInsname(deviceId);
                mH.sendMessage(mH.obtainMessage(RET_INSTANCE_DEL, payload));
            }
            break;
        case 4: /* HB_DEVICE_STATUS_UNKOWN_ERROR */
            {
                mainHandler().sendMessage(
                    mainHandler().obtainMessage(MT_DEVICE, DEVICE_STATUS_UNKOWN, 0));
            }
            break;
        case 0: /* HB_DEVICE_STATUS_UNINITIALIZED */
        case 1: /* HB_DEVICE_STATUS_INITIALIZING */
        default:
            break;
    }
}

void DeviceDataChannel::propertyChanged(const std::string &deviceId, const std::string &proKey, const std::string &proVal)
{
    LOGT("(%s, %s, %s)\n", deviceId.c_str(), proKey.c_str(), proVal.c_str());
    std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
    payload->mInsName = innerOfInsname(deviceId);
    payload->mSlots.push_back(InstancePayload::SlotInfo(proKey, proVal));
    mH.sendMessage(mH.obtainMessage(RET_INSTANCE_PUT, payload));
}

bool DeviceDataChannel::send(int action, std::shared_ptr<Payload> data)
{
    if (action == PT_INSTANCE_PAYLOAD) {
        std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(data));
        if (payload) {
            LOGI("setPropertyValue(%s, %s, %s)\n",
                outerOfInsname(payload->mInsName).c_str(),
                payload->mSlots[0].nName.c_str(),
                payload->mSlots[0].nValue.c_str());
            deviceManager().SetDevicePropertyValue(
                outerOfInsname(payload->mInsName),
                payload->mSlots[0].nName,
                payload->mSlots[0].nValue, true);
        }
    }
    return true;
}

ElinkDeviceDataChannel::ElinkDeviceDataChannel()
{
}

ElinkDeviceDataChannel::~ElinkDeviceDataChannel()
{
}

int ElinkDeviceDataChannel::init()
{
    DeviceDataChannel::init();

    /* regist device profile sync callback */
#ifdef SIM_SUITE
    deviceManager().mCallback = this;
#endif
    LOGI("init end\n");
    return 0;
}

} /* namespace HB */

