/***************************************************************************
 *  RuleEngineService.cpp - Rule Engine Service
 *
 *  Created: 2018-06-13 09:46:25
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineService.h"
#include "DataChannel.h"
#include "InstancePayload.h"
#include "ClassPayload.h"
#include "RulePayload.h"
#include "StringData.h"
#include "StringArray.h"
#include "Message.h"
#include "Common.h"
#include "RuleEngineLog.h"
#include "RuleEngineTimer.h"

#include <unistd.h>

#define RULE_DB_NAME "ruleengine.db"

namespace HB {

static RuleEngineService *gRuleEngine = 0;

RuleEngineService::RuleEngineService()
    : mServerRoot("clips")
    , mCoreForNormal(0), mCoreForUrgent(0) //, mStore(0)
    , mRuleChannel(0), mClassChannel(0)
    , mEnableRefreshRule(false)
{
}

RuleEngineService::~RuleEngineService()
{
    for (auto it = mOfflineInsesCalled.begin(); it != mOfflineInsesCalled.end(); ++it)
        it->second.clear();
    mOfflineInsesCalled.clear();
    mCoreForNormal.reset();
    mCoreForUrgent.reset();
    // mStore.reset();
    // mTimer.reset();
}

void RuleEngineService::setRuleChannel(std::shared_ptr<DataChannel> channel)
{
    mRuleChannel = channel;
}

void RuleEngineService::setDeviceChannel(std::shared_ptr<DataChannel> channel)
{
    mClassChannel = channel;
}

int RuleEngineService::init(const std::vector<DeviceTableInfo> &devices, const std::vector<RuleTableInfo> &rules, bool urgent)
{
    /*
    mStore = std::make_shared<RuleEngineStore>(mServerRoot + "/" + RULE_DB_NAME);
    mTimer = std::make_shared<RuleEngineTimer>(ruleHandler());

    size_t i,j;
    int eID;
    bool wflg;
    std::vector<std::string> eIDs;
    TimerEventInfo eInfo;
    std::vector<DefRuleInfo> ruleInfos = store()->queryRuleInfos();
    for (i = 10; i < ruleInfos.size(); ++i) {
        eIDs = stringSplit(ruleInfos[i].mTimers, ";");
        for (j = 0; j < eIDs.size(); ++j) {
            eID = atoi(eIDs[i].c_str());
            if (!store()->queryTimerEvent(eID, eInfo))
                continue;
            wflg = eInfo.mWeek.empty() ? true : false;
            TimerEvent::pointer tePtr = std::make_shared<TimerEvent>(eID, wflg);
            tePtr->year()->resetFromString(eInfo.mYear);
            tePtr->month()->resetFromString(eInfo.mMonth);
            if (wflg)
                tePtr->week()->resetFromString(eInfo.mWeek);
            else
                tePtr->day()->resetFromString(eInfo.mDay);
            tePtr->hour()->resetFromString(eInfo.mHour);
            tePtr->minute()->resetFromString(eInfo.mMinute);
            tePtr->second()->resetFromString(eInfo.mSecond);
            timer()->addEvent(ruleInfos[i].mDefName, tePtr);
        }
    }
    */

    /*{{{ normal rule trigger */
    ruleHandler().mCallback = this;
    mCoreForNormal = std::make_shared<RuleEngineCore>(ruleHandler(), "core1");
    mCoreForNormal->setup(
        std::make_shared<MsgPushCall>(this, &RuleEngineService::callMessagePush),
        std::make_shared<InsPushCall>(this, &RuleEngineService::callInstancePush),
        std::make_shared<TxtPushCall>(this, &RuleEngineService::callContentPush)
        );
    mCoreForNormal->start(mServerRoot, std::bind(&RuleEngineService::callGetFiles, this, std::placeholders::_1, false));
    /*}}}*/

    /*{{{ urgent rule trigger */
    if (urgent) {
        urgentHandler().mCallback = this;
        mCoreForUrgent = std::make_shared<RuleEngineCore>(urgentHandler(), "core2");
        mCoreForUrgent->setup(
            std::make_shared<MsgPushCall>(this, &RuleEngineService::callMessagePush),
            std::make_shared<InsPushCall>(this, &RuleEngineService::callInstancePush),
            std::make_shared<TxtPushCall>(this, &RuleEngineService::callContentPush)
            );
        mCoreForUrgent->start(mServerRoot, std::bind(&RuleEngineService::callGetFiles, this, std::placeholders::_1, true));
    }
    /*}}}*/

    if (mRuleChannel)
        mRuleChannel->init();
    if (mClassChannel)
        mClassChannel->init();

    for (size_t i = 0; i < devices.size(); ++i) {
        mCoreForNormal->handleClassSync(
            devices[i].nDeviceType.c_str(),
            devices[i].nScriptData.c_str());
    }

    /* build rule */
    for (size_t i = 0; i < rules.size(); ++i) {
        mCoreForNormal->handleRuleSync(
            rules[i].nRuleId.c_str(),
            rules[i].nScriptData.c_str());
    }
    // mCoreForNormal->reset(); // TODO call this will unkown crash
    ruleHandler().sendEmptyMessage(RET_REFRESH_TIMER);
    return 0;
}

bool RuleEngineService::handleMessage(Message *msg)
{
    if (msg->what == RET_REFRESH_TIMER)
        return ccore()->handleTimer();

    RE_LOGT("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    std::string assert;
    switch(msg->what) {
#if 0
        case RET_CLASS_SYNC:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<ClassPayload> payload(std::dynamic_pointer_cast<ClassPayload>(msg->obj));
                if (PT_CLASS_PAYLOAD != payload->type()) {
                    RE_LOGW("payload type not match!\n");
                    return false;
                }
                std::string path = ccore()->handleClassSync(
                    payload->mClsName.c_str(),
                    payload->toString().c_str());

                if (!path.empty()) {
                    std::size_t found = path.find_last_of("/");
                    store()->updateClassTable(payload->mClsName, payload->mVersion, path.substr(found+1));
                }
            }
            return true;/*}}}*/
        case RET_RULE_ADD:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<StringArray> ruledata(std::dynamic_pointer_cast<StringArray>(msg->obj));
                if (ruledata->size() != 3)
                    return false;
                std::string rul = ruledata->get(0);
                std::string ver = ruledata->get(1);
                std::string str = ruledata->get(2);
                std::string path = ccore()->handleRuleSync(rul.c_str(), str.c_str());
                if (path.empty())
                    return false;

                std::size_t found = path.find_last_of("/");
                store()->updateRuleTable(rul, ver, path.substr(found+1));
            }
            return true;/*}}}*/
        case RET_RULE_SYNC:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<RulePayload> payload(std::dynamic_pointer_cast<RulePayload>(msg->obj));
                if (PT_RULE_PAYLOAD != payload->type()) {
                    RE_LOGW("payload type not match!\n");
                    return false;
                }
                /* build rule construct in core clips */
                std::string path = ccore()->handleRuleSync(
                    payload->mRuleID.c_str(),
                    payload->toString().c_str());
                if (path.empty())
                    return false;
                /* update sqlite db */
                std::size_t found = path.find_last_of("/");
                store()->updateRuleTable(payload->mRuleID, payload->mVersion, path.substr(found+1));
                std::string eids;
                TimerEvent::pointer eptr;
                /* timer events of the rule */
                for (size_t i = 0; i < payload->mTimerEvents.size(); ++i) {
                    TimerEventInfo info;
                    eptr = std::dynamic_pointer_cast<TimerEvent>(payload->mTimerEvents[i]);
                    info.mID = eptr->getID();
                    info.mYear = eptr->year()->toString();
                    info.mMonth = eptr->month()->toString();
                    if (eptr->getFlag())
                        info.mWeek = eptr->week()->toString();
                    else
                        info.mDay = eptr->day()->toString();
                    info.mHour = eptr->hour()->toString();
                    info.mMinute = eptr->minute()->toString();
                    info.mSecond = eptr->second()->toString();
                    /* modify table of TimerEvent */
                    store()->updateTimerEvent(info);
                    /* add event to timer */
                    timer()->addEvent(payload->mRuleID, eptr);
                    eids.append(int2String(eptr->getID())).append(";");
                }
                /* modify table of DefRule for timers */
                store()->updateRuleForTimers(payload->mRuleID, eids);
                /* check enable of the rule */
                if (payload->mEnable)
                    enableRule(payload->mRuleID);
                else
                    disableRule(payload->mRuleID);
            }
            return true;/*}}}*/
        case RET_RULE_DELETE:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<StringData> ruleId(std::dynamic_pointer_cast<StringData>(msg->obj));
                /* retract rule */
                ccore()->deleteRule(ruleId->getData());
                /* stop timers of the rule */
                timer()->stop(ruleId->getData());
                /* delete rule from db */
                store()->deleteRule(ruleId->getData());
            }
            return true;/*}}}*/
#endif
        case RET_INSTANCE_ADD:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type()) {
                    if (ccore()->handleInstanceAdd(payload->mInsName.c_str(), payload->mClsName.c_str()))
                        _OnlineInstanceRefreshRules(payload->mInsName);
                }
            }
            return true;/*}}}*/
        case RET_INSTANCE_DEL:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type())
                    ccore()->handleInstanceDel(payload->mInsName.c_str());
            }
            return true;/*}}}*/
        case RET_INSTANCE_PUT:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type()) {
                    ccore()->handleInstancePut(
                        payload->mInsName.c_str(),
                        payload->mSlots[0].nName.c_str(),
                        payload->mSlots[0].nValue.c_str());
                }
            }
            return true;/*}}}*/
        case RET_ASSERT_FACT:/*{{{*/
            if (msg->obj) {
                std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
                ccore()->assertRun(data->getData());
                data->mMagic = msg->arg2;
            }
            return true;/*}}}*/
        case RET_SWITCH_RULE:/*{{{*/
            switch (msg->arg1) {
                case ENUM_ENABLE:
                    if (msg->obj) {
                        std::shared_ptr<StringArray> data(std::dynamic_pointer_cast<StringArray>(msg->obj));
                        if (ccore()->enableRule(data->get(0), data->get(1)))
                            data->mMagic = msg->arg2;
                    }
                    break;
                case ENUM_DISABLE:
                    if (msg->obj) {
                        std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
                        if (ccore()->disableRule(data->getData()))
                            data->mMagic = msg->arg2;
                    }
                    break;
                default:
                    return false;
            }
            return true;/*}}}*/
#if 0
        case RET_TIMER_EVENT:/*{{{*/
            switch (msg->arg1) {
                case TIMER_TOPLAY:
                    assert.append("(timer-event (id ").append(int2String(msg->arg2)).append("))");
                    break;
                case TIMER_DURATION:
                    assert.append("(remove-timer-event ").append(int2String(msg->arg2)).append(")");
                    break;
                default:
                    return false;
            }
            ccore()->assertRun(assert.c_str());
            return true;/*}}}*/
#endif
        default:
            return false;
    }
    return false;
}

bool RuleEngineService::_OfflineInstanceCalledByRHS(std::string &insName, std::string &ruleId)
{
    RE_LOGTT();
    if (!mEnableRefreshRule)
        return false;
    std::map<std::string, std::set<std::string>>::iterator it = mOfflineInsesCalled.find(insName);
    if (it != mOfflineInsesCalled.end()) {
        it->second.insert(ruleId);
        return true;
    }
    std::set<std::string> rules;
    rules.insert(ruleId);
    mOfflineInsesCalled.insert(std::pair<std::string, std::set<std::string>>(insName, rules));
    return true;
}

bool RuleEngineService::_OnlineInstanceRefreshRules(std::string &insName)
{
    RE_LOGTT();
    if (!mEnableRefreshRule)
        return false;
    std::map<std::string, std::set<std::string>>::iterator it = mOfflineInsesCalled.find(insName);
    if (it == mOfflineInsesCalled.end())
        return true;
    std::set<std::string>::iterator si;
    for ( si = it->second.begin(); si != it->second.end(); ++si)
        mCoreForNormal->refreshRule((*si).c_str());
    return true;
}

bool RuleEngineService::callMessagePush(int what, int arg1, std::string arg2, std::string message)
{
    RE_LOGI("(%d, %d, %s, %s)\n", what, arg1, arg2.c_str(), message.c_str());
    switch (what) {
        case MSG_RULE_RESPONSE:
            switch (arg1) {/*{{{ arg2: ruleid, message: detail info */
                case RUL_SUCCESS:
                    break;
                case RUL_FAIL:
                    break;
                case RUL_TIMEOUT:
                    break;
            }/*}}}*/
            break;
        case MSG_RULE_RHS:
            switch (arg1) {/*{{{ arg2: ruleid, message: instanceid */
                case RHS_INS_NOT_FOUND:
                    return _OfflineInstanceCalledByRHS(message, arg2);
                case RHS_NTF_WRONG_TYPE:
                    break;
                case RHS_SEE_NOT_FOUND:
                    break;
            }/*}}}*/
            break;
    }
    return false;
}

bool RuleEngineService::callInstancePush(std::string insName, std::string slot, std::string value)
{
    if ('#' == value[0])
        value = value.substr(1);
    RE_LOGI("(%s, %s, %s)\n", insName.c_str(), slot.c_str(), value.c_str());

    std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
    payload->mInsName = insName;
    payload->mSlots.push_back(InstancePayload::SlotInfo(slot, value));
    mClassChannel->send(PT_INSTANCE_PAYLOAD, payload);
    return false; /* asynchronous */
}

bool RuleEngineService::callContentPush(std::string id, std::string title, std::string content)
{
    RE_LOGW("(%s, %s, %s)\n", id.c_str(), title.c_str(), content.c_str());
    return true; /* synchronous */
}

bool RuleEngineService::triggerRule(const std::string &ruleId, bool sync)
{
    RE_LOGI("(%s)\n", ruleId.c_str());
    std::string assert("");
    assert.append("(rule ").append(innerOfRulename(ruleId)).append(")");
    std::shared_ptr<StringData> data = std::make_shared<StringData>(assert.c_str());
    ruleHandler().sendMessage(ruleHandler().obtainMessage(RET_ASSERT_FACT, 0, 99, data));
    if (sync) {
        for (int i = 0; i < 5; ++i) {
            if (data->mMagic == 99)
                return true;
            usleep(20000);
        }
        return false;
    }
    return true;
}

bool RuleEngineService::enableRule(const std::string &ruleId, const std::string &script, bool sync)
{
    RE_LOGI("(%s)\n", ruleId.c_str());
    std::shared_ptr<StringArray> data = std::make_shared<StringArray>();
    data->put(0, innerOfRulename(ruleId).c_str());
    data->put(1, script.c_str());
    ruleHandler().sendMessage(ruleHandler().obtainMessage(RET_SWITCH_RULE, ENUM_ENABLE, 99, data));
    if (sync) {
        for (int i = 0; i < 5; ++i) {
            if (data->mMagic == 99)
                return true;
            usleep(20000);
        }
        return false;
    }
    return true;
}

bool RuleEngineService::disableRule(const std::string &ruleId, bool sync)
{
    RE_LOGI("(%s)\n", ruleId.c_str());
    std::shared_ptr<StringData> data = std::make_shared<StringData>(innerOfRulename(ruleId).c_str());
    ruleHandler().sendMessage(ruleHandler().obtainMessage(RET_SWITCH_RULE, ENUM_DISABLE, 99, data));
    if (sync) {
        for (int i = 0; i < 5; ++i) {
            if (data->mMagic == 99)
                return true;
            usleep(20000);
        }
        return false;
    }
    return true;
}

std::vector<std::string> RuleEngineService::callGetFiles(int /*fileType*/, bool /*urgent*/)
{
    std::vector<std::string> files;
#if 0
    switch (fileType) {
        case TYPE_TEM_FILE:
            files = store()->queryTemplateFilePaths(urgent);
            break;
        case TYPE_CLS_FILE:
            files = store()->queryClassFilePaths(urgent);
            break;
        case TYPE_RUL_FILE:
            files = store()->queryRuleFilePaths(urgent);
            break;
        default:
            break;
    }
#endif
    return std::move(files);
}

void RuleEngineService::debug(int show, bool urgent)
{
    if (urgent)
        urgentHandler().sendMessage(urgentHandler().obtainMessage(
                RET_DEBUG, show, 0));
    else
        ruleHandler().sendMessage(ruleHandler().obtainMessage(
                RET_DEBUG, show, 0));
}

bool RuleEngineService::getRuleSwitch(const std::string &/*ruleName*/)
{
#if 0
    // TODO deprecate
    std::vector<DefRuleInfo> infos = store()->queryRuleInfos();
    for (size_t i = 0; i < infos.size(); ++i) {
        if (infos[i].mDefName == ruleName)
            return infos[i].mEnable ? true : false;
    }
#endif
    return false;
}

std::string RuleEngineService::getRuleScript(const std::string &ruleName)
{
    return ccore()->getRuleScript(ruleName.c_str());
}

std::vector<std::string> RuleEngineService::getRules()
{
    return ccore()->getRuleNames("rul-");
}

std::vector<std::string> RuleEngineService::getDevices()
{
    return ccore()->getClassNames("UUID");
}

std::vector<std::string> RuleEngineService::getSlots(const std::string &clsName)
{
    return ccore()->getSlotNames(clsName.c_str());
}

std::vector<std::string> RuleEngineService::getInstaces(const std::string &clsName)
{
    return ccore()->getObjectNames(clsName.c_str());
}

std::string RuleEngineService::getInstanceValue(const std::string &insName, const std::string &slotName)
{
    return ccore()->getObjectValue(insName.c_str(), slotName.c_str());
}

RuleEngineService& ruleEngine()
{
    if (0 == gRuleEngine)
        gRuleEngine = new RuleEngineService();
    return *gRuleEngine;
}


} /* namespace HB */

