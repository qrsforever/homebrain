/***************************************************************************
 *  LFUnitTest.h -
 *
 *  Created: 2019-07-11 10:17:44
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LFUnitTest_H__
#define __LFUnitTest_H__

#include "GatewayAgent.h"

#ifdef __cplusplus

namespace HB {

class UnittestCallback : public RemoteEventCallback {
public:
    virtual ~UnittestCallback() {}

    int doRemoteConnectSuccess() { return 0; }
    int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId);
    int doTopoAddReply(std::string elinkId, int code, std::string message);
    int doTopoDelReply(std::string elinkId, int code, std::string message);
    int doPropertySet(std::string elinkId, std::vector<struct DeviceProperty> &params);
    int doPropertyGet(std::string elinkId, std::vector<std::string> &params);
    int doUpgradeReply(std::string newVersion, int code, std::string message);
    int doUpgradeCheck(std::string text);
};

bool onLFUnitTest(int arg1, int arg2);

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __LFUnitTest_H__ */
