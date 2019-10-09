/***************************************************************************
 *  ScenePayload.cpp -
 *
 *  Created: 2019-06-10 15:09:23
 *
 *  Copyright QRS
 ****************************************************************************/

#include "ScenePayload.h"
#include "Common.h"
#include <vector>

namespace HB {

using namespace UTILS;

std::string MicroService::fullName()
{
    std::string tmp;
    tmp.append(mName).append(":").append(mId);
    return tmp;
}

std::string MicroService::makeStory(std::string fmt, std::string &svr)
{
    std::string sname = fullName();
    std::vector<std::string> deps = stringSplit(mDeps, ",");
    std::string tmp;
    tmp.append(fmt).append("(if (and");
    tmp.append(fmt).append("     (eq (member$ ").append(sname).append(" $?zmbs").append(") FALSE)");
    tmp.append(fmt).append("     (eq (member$ ").append(sname).append(" $?svrs").append(") FALSE)");
    for (size_t i = 0, l = deps.size(); i < l; ++i) {
        tmp.append(fmt).append("     (neq (member$ ").append(deps[i]).append(" $?zmbs").append(") FALSE)");
    }
    tmp.append(fmt).append("    )");
    tmp.append(fmt).append(" then").append(fmt).append("  ").append(svr);
    tmp.append(fmt).append(")");
    // tmp.append(fmt).append("(logd $?svrs)");
    // tmp.append(fmt).append("(logd $?zmbs)");
    return std::move(tmp);
}

std::string AutoLightMS::getConditions(std::string fmt)
{
    std::string tmp(fmt);
    tmp.append("   (object (is-a oic.d.light)\n");
    tmp.append("     (ID ").append(mLightUUID).append("))\n");
    tmp.append("   (object (is-a oic.d.envsensor)\n");
    tmp.append("     (ID ").append(mSensorUUID).append("))\n");
    return std::move(tmp);
}

std::string AutoLightMS::makeService(std::string fmt)
{
    std::string tmp;
    tmp.append("(assert (start-service ?sct ").append(fullName());
    tmp.append(" ").append(mSensorUUID).append(" ");
    tmp.append(mLightUUID).append(" ").append(mSensorIll).append(" ");
    tmp.append(mLightStep).append("))\n");
    return std::move(makeStory(fmt, tmp));
}

std::string GradLightMS::getConditions(std::string fmt)
{
    std::string tmp(fmt);
    tmp.append("   (object (is-a oic.d.light)\n");
    tmp.append("     (ID ").append(mLightUUID).append("))\n");
    return std::move(tmp);
}

std::string GradLightMS::makeService(std::string fmt)
{
    std::string tmp;
    tmp.append("(assert (start-service ?sct ").append(fullName());
    tmp.append(" ").append(mDirection).append(" ");
    tmp.append(mLightUUID).append(" ").append(mStepCount).append(" ");
    tmp.append(mTimeSecondMs).append("))\n");
    return std::move(makeStory(fmt, tmp));
}

std::string SosAlarmMS::getConditions(std::string fmt)
{
    std::string tmp(fmt);
    tmp.append("   (object (is-a oic.d.sosalarm)\n");
    tmp.append("     (ID ").append(mSosUUID).append("))\n");
    tmp.append("   (object (is-a oic.d.alarmer)\n");
    tmp.append("     (ID ").append(mAlarmUUID).append("))\n");
    tmp.append("   (object (is-a oic.d.light)\n");
    tmp.append("     (ID ").append(mLightUUID).append("))\n");
    return std::move(tmp);
}

std::string SosAlarmMS::makeService(std::string fmt)
{
    std::string tmp;
    tmp.append("(assert (start-service ?sct ").append(fullName());
    tmp.append(" ").append(mSosUUID).append(" ");
    tmp.append(mAlarmUUID).append(" ").append(mLightUUID).append(" ");
    tmp.append(mTimeSecondMs).append("))\n");
    return std::move(makeStory(fmt, tmp));
}

std::string DeviceControl::toString(std::string fmt)
{
    std::string tmp(fmt);
    tmp.append("(").append(ACT_CONTROL_FUNC).append(" ").append(innerOfInsname(mDid));
    tmp.append(" ").append(mSlot).append(" ").append(mValue).append(")\n");
    return std::move(tmp);
}

std::string NotifyContent::toString(std::string fmt)
{
    std::string tmp(fmt);
    tmp.append("(").append(ACT_NOTIFY_FUNC).append(" ").append(mID);
    tmp.append(" ").append(mTitle).append(" ").append(mContent).append(")\n");
    return std::move(tmp);
}

ScenePayload::ScenePayload(std::string id, std::string name)
    : mSid(id), mName(name), mRoom("default")
{

}

ScenePayload::~ScenePayload()
{
    for (size_t k = eON_ENTER; k <= eON_STORY; ++k) {
        for (size_t i = 0, l = mServices[k].size(); i < l; ++i)
            delete mServices[k][i];
        mServices[k].clear();

        for (size_t i = 0, l = mControls[k].size(); i < l; ++i)
            delete mControls[k][i];
        mControls[k].clear();

        for (size_t i = 0, l = mNotifies[k].size(); i < l; ++i)
            delete mNotifies[k][i];
        mNotifies[k].clear();
    }
}

void ScenePayload::addAutoLightMS(ms_on_event_t event, std::string id, std::string deps, std::string lightUUID,
    std::string lightStep, std::string sensorUUID, std::string sensorIll)
{
    AutoLightMS *ms = new AutoLightMS(id, "autolight", deps);
    if (ms == 0)
        return;
    ms->mLightUUID = innerOfInsname(lightUUID);
    ms->mLightStep = lightStep;
    ms->mSensorUUID = innerOfInsname(sensorUUID);
    ms->mSensorIll = sensorIll;
    mServices[event].push_back(ms);
}

void ScenePayload::addGradLightMS(ms_on_event_t event, std::string id, std::string deps, std::string lightUUID,
    std::string direction, std::string stepCount, std::string timeSecondMs)
{
    GradLightMS *ms = new GradLightMS(id, "gradlight", deps);
    if (ms == 0)
        return;
    ms->mLightUUID = innerOfInsname(lightUUID);
    ms->mDirection = direction;
    ms->mStepCount = stepCount;
    ms->mTimeSecondMs = timeSecondMs;
    mServices[event].push_back(ms);
}

void ScenePayload::addSosAlarmMS(ms_on_event_t event, std::string id, std::string deps, std::string sosUUID,
    std::string alarmUUID, std::string lightUUID, std::string timeSecondMs)
{
    SosAlarmMS *ms = new SosAlarmMS (id, "sosalarm", deps);
    if (ms == 0)
        return;
    ms->mSosUUID = innerOfInsname(sosUUID);
    ms->mAlarmUUID = innerOfInsname(alarmUUID);
    ms->mLightUUID = innerOfInsname(lightUUID);
    ms->mTimeSecondMs = timeSecondMs;
    mServices[event].push_back(ms);
}

void ScenePayload::addDeviceCtrl(ms_on_event_t event, std::string did, std::string pro, std::string val)
{
    DeviceControl *ctl = new DeviceControl(did, pro, val);
    if (ctl == 0)
        return;
    mControls[event].push_back(ctl);
}

void ScenePayload::addNotifyContent(ms_on_event_t event, std::string id, std::string title, std::string content)
{
    std::string tid;
    tid.append("\"").append(id).append("\"");
    std::string ttitle;
    ttitle.append("\"").append(title).append("\"");
    std::string tcontent;
    tcontent.append("\"").append(content).append("\"");
    NotifyContent *ntf = new NotifyContent(tid, ttitle, tcontent);
    if (ntf  == 0)
        return;
    mNotifies[event].push_back(ntf);
}

std::string ScenePayload::toString()
{
    std::string tmp;
    tmp.append("(defrule ").append(innerOfRulename(mSid)).append(" \"").append(mName).append("\"\n");
    // scene context
    tmp.append("   ?sct <- (object (is-a SceneContext)");
    tmp.append(" (where ").append(mRoom).append(") (target ").append(mName).append(") (action ?action)");
    tmp.append(" (services $?svrs) (zombies $?zmbs))");
    // scene condition
    // for (size_t k = eON_ENTER; k <= eON_STORY; ++k) {
    //     for (size_t i = 0, l = mServices[k].size(); i < l; ++i)
    //         tmp.append(mServices[k][i]->getConditions("\n"));
    // }
    tmp.append("\n  =>\n");

    // action == exit
    tmp.append("   (if (eq ?action exit)\n");
    tmp.append("     then\n");
    tmp.append("       (logd \"scene ").append(mName).append(" onexit!\")\n");
    tmp.append("       (foreach ?sname (create$ $?svrs)\n");
    tmp.append("           (logd \"stop service: \" ?sname)\n");
    tmp.append("           (assert (stop-service ?sname))\n");
    tmp.append("       )\n");
    for (size_t i = 0, l = mNotifies[eON_EXIT].size(); i < l; ++i)
        tmp.append(mNotifies[eON_EXIT][i]->toString("       "));
    for (size_t i = 0, l = mControls[eON_EXIT].size(); i < l; ++i)
        tmp.append(mControls[eON_EXIT][i]->toString("       "));
    tmp.append("       (return)\n");
    tmp.append("   )\n");

    // action == enter
    tmp.append("   (if (eq ?action enter)\n");
    tmp.append("     then\n");
    tmp.append("       (logd \"scene ").append(mName).append(" onenter!\")\n");
    for (size_t i = 0, l = mControls[eON_ENTER].size(); i < l; ++i)
        tmp.append(mControls[eON_ENTER][i]->toString("      "));
    for (size_t i = 0, l = mNotifies[eON_ENTER].size(); i < l; ++i)
        tmp.append(mNotifies[eON_ENTER][i]->toString("       "));
    tmp.append("       (return)\n");
    tmp.append("   )\n");

    // action == story
    tmp.append("   (if (eq ?action story)\n");
    tmp.append("     then\n");
    tmp.append("       (logd \"scene ").append(mName).append(" onstory!\")\n");
    for (size_t i = 0, l = mServices[eON_STORY].size(); i < l; ++i)
        tmp.append(mServices[eON_STORY][i]->makeService("\n      "));
    tmp.append("       (return)\n");
    tmp.append("   )\n");
    tmp.append(")");
    printf("scene rule: \n %s", tmp.c_str());
    return std::move(tmp);
}

} /* namespace HB */
