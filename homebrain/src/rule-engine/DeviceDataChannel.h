/***************************************************************************
 *  DeviceDataChannel.h - Device Data Channel
 *
 *  Created: 2018-06-14 15:04:25
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __DeviceDataChannel_H__
#define __DeviceDataChannel_H__

#include "DataChannel.h"
#include "HBHelper.h"

#ifdef __cplusplus

namespace HB {

class RuleEventHandler;

class DeviceDataChannel : public DataChannel {
public:
    DeviceDataChannel();
    ~DeviceDataChannel();

    int init();

    virtual void statusChanged(const std::string &deviceId, const std::string &deviceType, int status);
    virtual void propertyChanged(const std::string &deviceId, const std::string &propertyKey, const std::string &value);

    virtual bool send(int action, std::shared_ptr<Payload> payload);

protected:
    RuleEventHandler &mH;
}; /* class DeviceDataChannel */

class ElinkDeviceDataChannel : public DeviceDataChannel {
public:
    ElinkDeviceDataChannel();
    ~ElinkDeviceDataChannel();

    int init();

private:

}; /* class ElinkDeviceDataChannel */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __DeviceDataChannel_H__ */
