/***************************************************************************
 *  RuleEngineService.h - Rule Engine Service Header
 *
 *  Created: 2018-06-13 09:24:58
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineService_H__
#define __RuleEngineService_H__

#include "MessageHandler.h"
#include "DataChannel.h"
#include "RuleEventThread.h"
#include "RuleEngineTimer.h"
#include "RuleEngineCore.h"
#include "RuleEventHandler.h"

#include <map>
#include <set>

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class RuleEngineService : public MessageHandler::Callback {
public:
    RuleEngineService();
    ~RuleEngineService();

    bool handleMessage(Message *msg);

    void setServerRoot(std::string rootDir) { mServerRoot = rootDir; }
    std::string& getServerRoot() { return mServerRoot; }

    int init(const std::map<std::string, std::string> &devices,
        const std::map<std::string, std::string> &rules, bool urgent = false);

    void setRuleChannel(std::shared_ptr<DataChannel> channel);
    void setDeviceChannel(std::shared_ptr<DataChannel> channel);
    void setNotifyChannel(std::shared_ptr<DataChannel> channel);

    bool callMessagePush(int what, int arg1, std::string arg2, std::string message);
    bool callInstancePush(std::string insName, std::string slot, std::string value);
    bool callContentPush(std::string id, std::string title, std::string content);

    bool triggerRule(const std::string &ruleId, bool sync = false);
    bool enableRule(const std::string &ruleId, const std::string &script, bool sync = false);
    bool disableRule(const std::string &ruleId, bool sync = false);
    bool assertRule(const std::string &assert, bool sync = false);
    bool executeScene(const std::string &sceneName, const std::string &room, bool sync = false);

    std::vector<std::string> callGetFiles(int fileType, bool urgent);

    RuleEngineCore::pointer coreForNormal() { return mCoreForNormal; }
    RuleEngineCore::pointer coreForUrgent() { return mCoreForUrgent; }

    bool isUrgent() { return (mCoreForUrgent && pthread_self() != ruleHandler().id()); }

    void debug(int show, bool urgent = false);

    /* for monitor tool debug, will delete */
    bool getRuleSwitch(const std::string &ruleName);
    std::string getRuleScript(const std::string &ruleName);
    std::vector<std::string> getRules();
    std::vector<std::string> getDevices();
    std::vector<std::string> getSlots(const std::string &clsName);
    std::vector<std::string> getInstaces(const std::string &clsName);
    std::string getInstanceValue(const std::string &insName, const std::string &slotName);

    void getEnvBuildInfo(std::string &ver, std::string &user, std::string &dt) {
        return ccore()->getEnvBuildInfo(ver, user, dt);
    }

private:
    friend class RuleEventHandler;
    inline RuleEngineCore::pointer ccore();
    // inline RuleEngineTimer::pointer timer() { return mTimer; }

    bool _OfflineInstanceCalledByRHS(std::string &insName, std::string &rulName);
    bool _OnlineInstanceRefreshRules(std::string &insName);

private:
    std::string mServerRoot;
    RuleEngineCore::pointer mCoreForNormal;
    RuleEngineCore::pointer mCoreForUrgent;

    RuleEngineTimer::pointer mTimer;
    DataChannel::pointer mRuleChannel;
    DataChannel::pointer mClassChannel;
    DataChannel::pointer mNotifyChannel;

    bool mEnableRefreshRule;
    std::map<std::string, std::set<std::string>> mOfflineInsesCalled;
}; /* class RuleEngineService */

RuleEngineCore::pointer RuleEngineService::ccore()
{
    if (isUrgent())
        return mCoreForUrgent;
    return mCoreForNormal;
}

RuleEngineService& ruleEngine();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineService_H__ */
