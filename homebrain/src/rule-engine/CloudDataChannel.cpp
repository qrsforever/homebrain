/***************************************************************************
 *  CloudDataChannel.cpp - Rule Data Channel
 *
 *  Created: 2018-06-14 15:08:33
 *
 *  Copyright QRS
 ****************************************************************************/

#include "CloudDataChannel.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "InstancePayload.h"
#include "Common.h"
#include "Log.h"

#ifndef SIM_SUITE
std::string getClassNameByDeviceId(const std::string &/*deviceId*/)
{
    /* TODO need Device Manager offer the api */
    return std::string("DEVICE");
}
#endif

using namespace UTILS;

namespace HB {

CloudDataChannel::CloudDataChannel()
    : mH(ruleHandler())
{
}

CloudDataChannel::~CloudDataChannel()
{
}

int CloudDataChannel::init()
{
    return 0;
}

bool CloudDataChannel::send(int action, std::shared_ptr<Payload> payload)
{
    (void)action;
    (void)payload;
    return false;
}

ElinkCloudDataChannel::ElinkCloudDataChannel()
{

}

ElinkCloudDataChannel::~ElinkCloudDataChannel()
{

}

int ElinkCloudDataChannel::init()
{/*{{{*/
    LOGTT();
#ifdef SIM_SUITE
    /* regist rule sync json doc from cloud */
    cloudManager().registSyncRuleProfileCallback(
        std::bind(
            &ElinkCloudDataChannel::onSyncRuleProfile,
            this,
            std::placeholders::_1)
        );
    cloudManager().registSyncDeviceProfileCallback(
        std::bind(
            &ElinkDeviceDataChannel::onSyncDeviceProfile,
            this,
            std::placeholders::_1,
            std::placeholders::_2)
        );
#endif
    return 0;
}/*}}}*/

void ElinkCloudDataChannel::onSyncRuleProfile(const std::string jsonDoc)
{/*{{{*/
    LOGI("jsonDoc:\n%s\n", jsonDoc.c_str());
    rapidjson::Document doc;
    doc.Parse<0>(jsonDoc.c_str());
    if (doc.HasParseError()) {
        rapidjson::ParseErrorCode code = doc.GetParseError();
        LOGE("rapidjson parse error[%d]\n", code);
        return;
    }

    if (!doc.HasMember("status") && !doc.HasMember("result")) {
        LOGE("rapidjson parse error!\n");
        return;
    }

    rapidjson::Value &result = doc["result"];
    if (!result.HasMember("rule")) {
        LOGE("rapidjson parse error, no rule key!\n");
        return;
    }

    rapidjson::Value &rule = result["rule"];
    if (!rule.IsObject()) {
        LOGE("rapidjson parse error, rule is not object!\n");
        return;
    }

    std::shared_ptr<RulePayload> payload = std::make_shared<RulePayload>();
    if (!parseRule(rule, payload)) {
        LOGE("rapidjson parse error, parse rule object!\n");
        return;
    }
    mH.sendMessage(mH.obtainMessage(RET_RULE_SYNC, payload));
}/*}}}*/

void ElinkCloudDataChannel::onSyncDeviceProfile(const std::string deviceName, const std::string jsonDoc)
{/*{{{*/
    // LOGI("jsondoc:\n%s\n", jsondoc.c_str());
    rapidjson::Document doc;
    doc.Parse<0>(jsonDoc.c_str());
    if (doc.HasParseError()) {
        rapidjson::ParseErrorCode code = doc.GetParseError();
        LOGE("rapidjson parse error[%d]\n", code);
        return;
    }

    if (!doc.HasMember("status") && !doc.HasMember("result")) {
        LOGE("rapidjson parse error!\n");
        return;
    }

    rapidjson::Value &result = doc["result"];
    if (!result.HasMember("profile")) {
        LOGE("rapidjson parse error, no profile key!\n");
        return;
    }

    rapidjson::Value &profile = result["profile"];

    if (!profile.IsObject()) {
        LOGE("parse profile error, not object!\n");
        return;
    }
    /* elink v0.0.7 */
    std::string profileVersion(ELINK_DEVICE_DOC_VERSION);

    std::shared_ptr<ClassPayload> payload = std::make_shared<ClassPayload>(deviceName, "DEVICE", profileVersion);
    if (!parseProfile(profile, payload)) {
        LOGE("rapidjson parse error, parse rule object!\n");
        return;
    }
    mH.sendMessage(mH.obtainMessage(RET_CLASS_SYNC, payload));
}/*}}}*/

bool ElinkCloudDataChannel::send(int action, std::shared_ptr<Payload> payload)
{/*{{{*/
    (void)action;
    (void)payload;
    return false;
}/*}}}*/

bool ElinkCloudDataChannel::parseProfile(rapidjson::Value &profile, std::shared_ptr<ClassPayload> payload)
{/*{{{*/
    rapidjson::Value::ConstMemberIterator itprofile;
    for (itprofile = profile.MemberBegin(); itprofile != profile.MemberEnd(); ++itprofile) {
        LOGI("%s is %d\n", itprofile->name.GetString(), itprofile->value.GetType());
        Slot &slot = payload->makeSlot(itprofile->name.GetString());
        if (!itprofile->value.IsObject()) {
            LOGE("parse profile %s error, value is not object!\n", itprofile->name.GetString());
            return false;
        }
        if (!itprofile->value.HasMember("type") && !itprofile->value.HasMember("range")) {
            LOGE("parse profile %s.type|range error, not found type!\n", itprofile->name.GetString());
            return false;
        }
        const rapidjson::Value &type = itprofile->value["type"];
        const rapidjson::Value &range = itprofile->value["range"];
        const char *typestr = type.GetString();
        if (!strncmp(typestr, "enum", 4)) {
            if (!range.IsObject()) {
                LOGE("parse profile %s.range error, not object!\n", itprofile->name.GetString());
                return false;
            }
            std::string allowlist;
            rapidjson::Value::ConstMemberIterator itrange;
            bool symb2int = true;
            for (itrange = range.MemberBegin(); itrange != range.MemberEnd(); ++itrange) {
                LOGI("range[%s]: %s is %d\n", itprofile->name.GetString(), itrange->name.GetString(), itrange->value.GetType());
                const char *enumrange = itrange->name.GetString();
                /* TODO */
                if (!isdigit(enumrange[0]))
                    symb2int = false;
                allowlist.append(" ").append(itrange->name.GetString());
            }
            if (symb2int)
                slot.mType = ST_NUMBER;
            else
                slot.mType = ST_SYMBOL;
            slot.mAllowList = stringTrim(allowlist);
        } else if (!strncmp(typestr, "boolean", 7)) {
            if (!range.IsObject()) {
                LOGE("parse profile %s.range error, not object!\n", itprofile->name.GetString());
                return false;
            }
            slot.mType = ST_NUMBER;
            slot.mAllowList = "0 1";
        } else if (!strncmp(typestr, "int", 3)) {
            if (!range.IsString()) {
                LOGE("parse profile %s.range error, not String!\n", itprofile->name.GetString());
                return false;
            }
            if (!parsePropValue(range.GetString(), slot)) {
                LOGE("parse profile %s.range error!\n", itprofile->name.GetString());
                return false;
            }
            slot.mType = ST_NUMBER;
        } else if (!strncmp(typestr, "float", 5)) {
            if (!range.IsString()) {
                LOGE("parse profile %s.range error, not String!\n", itprofile->name.GetString());
                return false;
            }
            if (!parsePropValue(range.GetString(), slot)) {
                LOGE("parse profile %s.range error!\n", itprofile->name.GetString());
                return false;
            }
            slot.mType = ST_NUMBER;
        } else {
            LOGE("parse profile error, not support type!\n");
            return false;
        }
    }
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseRule(rapidjson::Value &rule, std::shared_ptr<RulePayload> payload)
{/*{{{*/
    /* parse header */
    rapidjson::Value &ruleName = rule["sceneName"];
    if (!ruleName.IsString()) {
        LOGE("parse rule name error!\n");
        return false;
    }
    rapidjson::Value &ruleId = rule["sceneId"];
    if (!ruleId.IsString() || ruleId.GetStringLength() < 6) {
        LOGE("parse rule id error!\n");
        return false;
    }

    std::string description("");
    rapidjson::Value &ruleDesr = rule["description"];
    if (ruleDesr.IsString())
        description.assign(ruleDesr.GetString());
    else
        description.assign(ruleName.GetString());

    rapidjson::Value &trigger = rule["trigger"];
    if (!trigger.IsObject()) {
        LOGE("parse trigger error!\n");
        return false;
    }

    payload->mRuleName = ruleName.GetString();
    payload->mRuleID = ruleId.GetString();
    payload->mRuleDesr = description;
    payload->mVersion = ELINK_RULE_DOC_VERSION;

    /* parse trigger */
    if (!parseTrigger(trigger, payload)) {
        LOGE("parse trigger object error!\n");
        return false;
    }

    /* parse conditions */
    if (rule.HasMember("conditions")) {
        rapidjson::Value &conditions = rule["conditions"];
        if (conditions.IsObject()) {
            if (!parseConditions(conditions, payload)) {
                LOGE("parse conditions object error!\n");
                return false;
            }
        }
    }

    /* parse actions */
    if (rule.HasMember("actions")) {
        rapidjson::Value &actions = rule["actions"];
        if (!actions.IsObject()) {
            LOGE("parse actions object error!\n");
            return false;
        }
        if (!parseActions(actions, payload)) {
            LOGE("parse actions object error!\n");
            return false;
        }
    }
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseTrigger(rapidjson::Value &trigger, std::shared_ptr<RulePayload> payload)
{/*{{{*/
    // rapidjson::Value &triggerType = trigger["triggerType"];
    // if (!triggerType.IsString()) {
    //     LOGE("parse triggerType error!\n");
    //     return false;
    // }
    //
    // if (strstr(triggerType.GetString(), "auto"))
    //     payload->mAuto = true;
    // else
    //     payload->mAuto = false;

    // if (strstr(triggerType.GetString(), "manual"))
    //     payload->mManual = true;
    // else
    //     payload->mManual = false;

    rapidjson::Value &nswitch = trigger["switch"];
    if (!nswitch.IsObject()) {
        LOGE("parse switch error!\n");
        return false;
    }
    rapidjson::Value &enabled = nswitch["enabled"];
    if (!enabled.IsString()) {
        LOGE("parse enabled error!\n");
        return false;
    }
    if (!strncmp(enabled.GetString(), "off", 3))
        payload->mEnable = false;
    else
        payload->mEnable = true;

    rapidjson::Value &timeCond = nswitch["timeCondition"];
    if (timeCond.IsString()) {
        if (!strncmp(timeCond.GetString(), "on", 2))
            payload->mTimeCondEnable = true;
    }

    rapidjson::Value &deviceCond = nswitch["deviceCondition"];
    if (deviceCond.IsString()) {
        if (!strncmp(deviceCond.GetString(), "on", 2))
            payload->mDevCondEnable = true;
    }

    rapidjson::Value &envCond = nswitch["environmentCondition"];
    if (envCond.IsString()) {
        if (!strncmp(envCond.GetString(), "on", 2))
            payload->mEnvCondEnable = true;
    }

    rapidjson::Value &notify = nswitch["notify"];
    if (notify.IsString()) {
        if (!strncmp(notify.GetString(), "on", 2))
            payload->mNotifyEnable = true;
    }

    rapidjson::Value &manual = nswitch["manual"];
    if (manual.IsString()) {
        if (!strncmp(manual.GetString(), "on", 2))
            payload->mManual = true;
        else
            payload->mManual = false;
    }

    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseTimeString(const char *ctimestr, SlotPoint &slotpoint)
{/*{{{*/
    if (!ctimestr || strlen(ctimestr) == 0)
        return true;
    char timestr[256] = { 0 };
    snprintf(timestr, 255, "%s", ctimestr);

    if (strchr(timestr, '|')) {
        char *t_or = strtok(timestr,"|");
        if (t_or) {
            slotpoint.mCellLogic = "|";
            do {
                slotpoint.append("=", t_or);
                t_or = strtok(NULL, "|");
            } while (t_or);
            return true;
        }
    }

    if (strchr(timestr, '-')) {
        char *t_range = strtok(timestr, "-");
        if (t_range) {
            slotpoint.mCellLogic = "&";
            slotpoint.append(">=", t_range);
            t_range = strtok(NULL, "-");
            if (!t_range) {
                LOGE("parse conditions.timeCondition.x error!\n");
                return false;
            }
            slotpoint.append("<=", t_range);
            return true;
        }
    }

    if (strchr(timestr, '!')) {
        char *t_not = strtok(timestr, "!");
        if (t_not) {
            slotpoint.mCellLogic = "|";
            do {
                slotpoint.append("<>", t_not);
                t_not = strtok(NULL, "!");
            } while (t_not);
            return true;
        }
    }

    if (!strncmp(timestr, "every", 5))
        return true;

    /* TODO */
    slotpoint.append("=", ctimestr);
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseScene(rapidjson::Value &jsondoc, std::shared_ptr<ScenePayload> payload)
{/*{{{*/
    rapidjson::Value &sceneName = jsondoc["sceneName"];
    if (!sceneName.IsString()) {
        LOGE("parse scene name error!\n");
        return false;
    }
    rapidjson::Value &sceneId = jsondoc["sceneId"];
    if (!sceneId.IsString() || sceneId.GetStringLength() < 1) {
        LOGE("parse scene id error!\n");
        return false;
    }
    rapidjson::Value &scene = jsondoc["scene"];
    if (!scene.IsObject()) {
        LOGE("parse scene.scene error!\n");
        return false;
    }
    bool flag = false;
    if (scene.HasMember("onEnter")) {
        rapidjson::Value &onEnter = scene["onEnter"];
        if (!onEnter.IsObject()) {
            LOGE("parse onEnter object error!\n");
            return false;
        }
        rapidjson::Value &deviceControl = onEnter["deviceControl"];
        if (deviceControl.IsArray() && deviceControl.Size() > 0) {
            if (!parseDeviceControl(deviceControl, eON_ENTER, payload)) {
                LOGE("parse onEnter.deviceControl error!\n");
                return false;
            }
        }
        rapidjson::Value &notifyContent = onEnter["notifyContent"];
        if (notifyContent.IsArray() && notifyContent.Size() > 0) {
            if (!parseNotifyContent(notifyContent, eON_ENTER, payload)) {
                LOGE("parse onEnter.notifyContent error!\n");
                return false;
            }
        }
        rapidjson::Value &microservice = onEnter["microService"];
        if (microservice.IsArray() && microservice.Size() > 0) {
            if (!parseMicroService(microservice, eON_ENTER, payload)) {
                LOGE("parse onEnter.microService error!\n");
                return false;
            }
        }
        flag = true;
    }

    if (scene.HasMember("onExit")) {
        rapidjson::Value &onExit = scene["onExit"];
        if (!onExit.IsObject()) {
            LOGE("parse onExit object error!\n");
            return false;
        }
        rapidjson::Value &deviceControl = onExit["deviceControl"];
        if (deviceControl.IsArray() && deviceControl.Size() > 0) {
            if (!parseDeviceControl(deviceControl, eON_EXIT, payload)) {
                LOGE("parse onExit.deviceControl error!\n");
                return false;
            }
        }
        rapidjson::Value &notifyContent = onExit["notifyContent"];
        if (notifyContent.IsArray() && notifyContent.Size() > 0) {
            if (!parseNotifyContent(notifyContent, eON_EXIT, payload)) {
                LOGE("parse onEnter.notifyContent error!\n");
                return false;
            }
        }
        flag = true;
    }

    if (scene.HasMember("onStory")) {
        rapidjson::Value &onStory = scene["onStory"];
        if (!onStory.IsObject()) {
            LOGE("parse onStory object error!\n");
            return false;
        }
        rapidjson::Value &microservice = onStory["microService"];
        if (microservice.IsArray() && microservice.Size() > 0) {
            if (!parseMicroService(microservice, eON_STORY, payload)) {
                LOGE("parse onStory.microService error!\n");
                return false;
            }
        }
        flag = true;
    }
    if (!flag) {
        LOGE("parse json error!\n");
        return false;
    }
    payload->mSid = sceneId.GetString();
    payload->mName = sceneName.GetString();
    payload->mRoom = "default";
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseDeviceControl(rapidjson::Value &controls, ms_on_event_t event, std::shared_ptr<ScenePayload> payload)
{/*{{{*/
    for (size_t i = 0; i < controls.Size(); ++i) {
        rapidjson::Value &ctl = controls[i];
        rapidjson::Value &did = ctl["deviceId"];
        rapidjson::Value &pro = ctl["propName"];
        rapidjson::Value &val = ctl["propValue"];
        if (!did.IsString() || !pro.IsString() || !val.IsString())
            return false;
        payload->addDeviceCtrl(event, did.GetString(), pro.GetString(), val.GetString());
    }
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseNotifyContent(rapidjson::Value &contents, ms_on_event_t event, std::shared_ptr<ScenePayload> payload)
{/*{{{*/
    for (size_t i = 0; i < contents.Size(); ++i) {
        rapidjson::Value &ntf = contents[i];
        rapidjson::Value &sceneId = ntf["sceneId"];
        rapidjson::Value &sceneName = ntf["sceneName"];
        rapidjson::Value &enable = ntf["enable"];
        if (!sceneId.IsString() || !enable.IsString() || !sceneName.IsString())
            return false;
        std::string nid(NOID_RULE_ENABLE);
        if (strncmp("1", enable.GetString(), 1))
            nid.assign(NOID_RULE_DISABLE);
        payload->addNotifyContent(event, nid, sceneId.GetString(), sceneName.GetString());
    }
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseMicroService(rapidjson::Value &services, ms_on_event_t event, std::shared_ptr<ScenePayload> payload)
{/*{{{*/
    for (size_t i = 0; i < services.Size(); ++i) {
        rapidjson::Value &svr = services[i];
        rapidjson::Value &name = svr["name"];
        rapidjson::Value &msid = svr["msid"];
        rapidjson::Value &deps = svr["deps"];
        if (!name.IsString() || !msid.IsString()) {
            LOGE("parse onEnter.microService[%d] error!\n", i);
            return false;
        }
        std::string depends("");
        if (!deps.IsNull() and deps.IsString())
            depends = deps.GetString();
        if (0 == strncmp("autolight", name.GetString(), 9)) {
            rapidjson::Value &params = svr["params"];
            if (!params.IsObject()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            rapidjson::Value &lightUUID = params["lightUUID"];
            rapidjson::Value &lightStep = params["lightStep"];
            rapidjson::Value &sensorUUID = params["sensorUUID"];
            rapidjson::Value &sensorIll = params["sensorIll"];
            if (!lightUUID.IsString() || !lightStep.IsString() ||
                !sensorUUID.IsString() || !sensorIll.IsString()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            payload->addAutoLightMS(event, msid.GetString(), depends, lightUUID.GetString(),
                lightStep.GetString(), sensorUUID.GetString(), sensorIll.GetString());
        } else if (0 == strncmp("gradlight", name.GetString(), 9)) {
            rapidjson::Value &params = svr["params"];
            if (!params.IsObject()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            rapidjson::Value &lightUUID = params["lightUUID"];
            rapidjson::Value &direction = params["direction"];
            rapidjson::Value &stepCount = params["stepCount"];
            rapidjson::Value &timeSeconds = params["timeSeconds"];
            if (!lightUUID.IsString() || !direction.IsString() ||
                !stepCount.IsString() || !timeSeconds.IsString()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            std::string timeMs = int2String(atoi(timeSeconds.GetString()) * 1000);
            payload->addGradLightMS(event, msid.GetString(), depends, lightUUID.GetString(),
                direction.GetString(), stepCount.GetString(), timeMs);
        } else if (0 == strncmp("sosalarm", name.GetString(), 8)) {
            rapidjson::Value &params = svr["params"];
            if (!params.IsObject()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            rapidjson::Value &sosUUID = params["sosUUID"];
            rapidjson::Value &alarmUUID = params["alarmUUID"];
            rapidjson::Value &lightUUID = params["lightUUID"];
            rapidjson::Value &timeSeconds = params["timeSeconds"];
            if (!sosUUID.IsString() || !alarmUUID.IsString() ||
                !lightUUID.IsString() || !timeSeconds.IsString()) {
                LOGE("parse [%d].microService[%d] params error!\n", event, i);
                return false;
            }
            std::string timeMs = int2String(atoi(timeSeconds.GetString()) * 1000);
            payload->addSosAlarmMS(event, msid.GetString(), depends, sosUUID.GetString(),
                alarmUUID.GetString(), lightUUID.GetString(), timeMs);
        } else {
            LOGE("parse not support microService[%s] error!\n", name.GetString());
            return false;
        }
    }
    return true;
}/*}}}*/

#ifdef USE_TIMER_EVENT
bool ElinkCloudDataChannel::parseTimeString(const char *ctimestr, TimeNode &node)
{/*{{{*/
    char *timestr = (char*)ctimestr;
    char *t_set = strtok(timestr,"|");
    if (t_set) {
        node.setValueType(eSet);
        do {
            node.append(atoi(t_set));
            t_set = strtok(NULL, "|");
        } while (t_set);
        return true;
    }

    char *t_min = strtok(timestr, "-");
    char *t_max = strtok(NULL, "-");
    if (t_min && t_max) {
        node.setValueType(eRange);
        node.setRange(atoi(t_min), atoi(t_max));
        return true;
    } else {
        LOGE("parse conditions.timeCondition.x error!\n");
        return false;
    }

    if (!strncmp(timestr, "every", 5)) {
        node.setValueType(eAny);
        return true;
    }

    char *t_not = strtok(timestr, "!");
    if (t_not) {
        node.setValueType(eNot);
        do {
            node.append(atoi(t_not));
            t_not = strtok(NULL, "!");
        } while (t_not);
        return true;
    }

    node.setValueType(eNull);
    return true;
}/*}}}*/
#endif

bool ElinkCloudDataChannel::parsePropValue(const char *cpropval, SlotPoint &slotpoint)
{/*{{{*/
    char *propval = (char*)cpropval;
    char *equ = strstr(propval, "==");
    if (equ) {
        if (strstr(equ + 2, "true"))
            slotpoint.append("=", "1");
        else if (strstr(equ + 2, "false"))
            slotpoint.append("=", "0");
        else
            slotpoint.append("=", equ + 2);
        return true;
    }
    char *egt = strstr(propval, ">=");
    if (egt) {
        slotpoint.append(">=", egt + 2);
        return true;
    }
    char *elt = strstr(propval, "<=");
    if (elt) {
        slotpoint.append("<=", elt + 2);
        return true;
    }
    char *neq = strstr(propval, "~=");
    if (neq) {
        slotpoint.append("<>", neq + 2);
        return true;
    }
    char *neg2 = strstr(propval, "!=");
    if (neg2) {
        slotpoint.append("<>", neg2 + 2);
        return true;
    }
    char *egu2 = strstr(propval, "=");
    if (egu2) {
        if (strstr(egu2 + 1, "true"))
            slotpoint.append("=", "1");
        else if (strstr(egu2 + 1, "false"))
            slotpoint.append("=", "0");
        else
            slotpoint.append("=", egu2 + 1);
        return true;
    }
    char *gt = strstr(propval, ">");
    if (gt) {
        slotpoint.append(">", gt + 1);
        return true;
    }
    char *lt = strstr(propval, "<");
    if (lt) {
        slotpoint.append("<", lt + 1);
        return true;
    }
    LOGE("unkown compare logic!\n");
    return false;
}/*}}}*/

bool ElinkCloudDataChannel::parseConditions(rapidjson::Value &conditions, std::shared_ptr<RulePayload> payload)
{/*{{{*/
    rapidjson::Value &conditionLogic = conditions["conditionLogic"];
    if (!conditionLogic.IsString()) {
        LOGE("parse conditions.conditionLogic error!\n");
        return false;
    }

    if (0 == conditionLogic.GetStringLength())
        payload->mLHS->mNodeLogic = "and";
    else
        payload->mLHS->mNodeLogic.assign(conditionLogic.GetString());

    if (conditions.HasMember("sceneCondition")) {
        rapidjson::Value &scene = conditions["sceneCondition"];
        if (scene.IsString() && scene.GetStringLength() > 1) {
            LHSNode *node = &(payload->mLHS->makeNode("and"));
            Condition &sceneCond = node->makeCond(CT_SCENE, "SceneContext", "noused");
            sceneCond.makeSlot("where").append("none", "default");
            sceneCond.makeSlot("target").append("none", scene.GetString());
        }
    }

    if (payload->mTimeCondEnable && conditions.HasMember("timeCondition")) {/*{{{*/
        rapidjson::Value &timeCondition = conditions["timeCondition"];
        if (timeCondition.IsArray() && timeCondition.Size() > 0) {
            LHSNode *node = &(payload->mLHS->makeNode("or"));
            char fctID[8] = { 0 };
            for (size_t i = 0; i < timeCondition.Size(); ++i) {
                rapidjson::Value &dt = timeCondition[i];
                sprintf(fctID, "fct_f%lu", i);
#ifdef USE_TIMER_EVENT
                char eID[9] = { 0 };
                sprintf(eID, "%u", rand() % 100000000);
                std::shared_ptr<TimerEvent> te;
                if (dt.HasMember("week"))
                    te = std::make_shared<TimerEvent>(atoi(eID), true);
                else
                    te = std::make_shared<TimerEvent>(atoi(eID), false);
                Condition &timeCond = node->makeCond(CT_TEMPLATE, "timer-event", fctID);
                timeCond.makeSlot("id").append("eq", eID);
#else
                Condition &timeCond = node->makeCond(CT_FACT, "datetime", fctID);
#endif

                if (!dt.IsObject()) {
                    LOGE("parse conditions.timeCondition[%d] error!\n", i);
                    return false;
                }
                timeCond.makeSlot("clock");
                if (dt.HasMember("year") && dt["year"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["year"].GetString(), *(te->year()))) {
#else
                    if (!parseTimeString(dt["year"].GetString(), timeCond.makeSlot("year"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].year error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("month") && dt["month"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["month"].GetString(), *(te->month()))) {
#else
                    if (!parseTimeString(dt["month"].GetString(), timeCond.makeSlot("month"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].month error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("day") && dt["day"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["day"].GetString(), *(te->day()))) {
#else
                    if (!parseTimeString(dt["day"].GetString(), timeCond.makeSlot("day"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].day error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("hour") && dt["hour"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["hour"].GetString(), *(te->hour()))) {
#else
                    if (!parseTimeString(dt["hour"].GetString(), timeCond.makeSlot("hour"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].hour error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("minute") && dt["minute"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["minute"].GetString(), *(te->minute()))) {
#else
                    if (!parseTimeString(dt["minute"].GetString(), timeCond.makeSlot("minute"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].minute error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("second") && dt["second"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["second"].GetString(), *(te->second()))) {
#else
                    if (!parseTimeString(dt["second"].GetString(), timeCond.makeSlot("second"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].second error!\n", i);
                        return false;
                    }
                }
                if (dt.HasMember("week") && dt["week"].IsString()) {
#ifdef USE_TIMER_EVENT
                    if (!parseTimeString(dt["week"].GetString(), *(te->week()))) {
#else
                    if (!parseTimeString(dt["week"].GetString(), timeCond.makeSlot("week"))) {
#endif
                        LOGE("parse conditions.timeCondition[%d].week error!\n", i);
                        return false;
                    }
                }
#ifdef USE_TIMER_EVENT
                payload->mTimerEvents.push_back(te);
#endif
            } /* end for */
        } /* end timeCondition */
    }/*}}}*/

    if (payload->mEnvCondEnable && conditions.HasMember("environmentCondition")) {/*{{{*/
        rapidjson::Value &envCondition = conditions["environmentCondition"];
        if (envCondition.IsObject()) {
            rapidjson::Value &envLogic = envCondition["environmentLogic"];
            if (!envLogic.IsString()) {
                LOGE("parse conditions.envCondition.envLogic error!\n");
                return false;
            }
            rapidjson::Value &envStatus = envCondition["environmentStatus"];
            if (!envStatus.IsArray()) {
                LOGE("parse conditions.envCondition.envStatus error!\n");
                return false;
            }
        } /* end envCondition */
        /* TODO  */

    } /*}}}*/

    if (payload->mDevCondEnable && conditions.HasMember("deviceCondition")) {/*{{{*/
        rapidjson::Value &deviceCondition = conditions["deviceCondition"];
        if (deviceCondition.IsObject()) {
            rapidjson::Value &deviceLogic = deviceCondition["deviceLogic"];
            if (!deviceLogic.IsString()) {
                LOGE("parse conditions.deviceCondition.deviceLogic error!\n");
                return false;
            }
            rapidjson::Value &deviceStatus = deviceCondition["deviceStatus"];
            if (!deviceStatus.IsArray() && deviceStatus.Size() > 0) {
                LOGE("parse conditions.deviceCondition.deviceStatus error!\n");
                return false;
            }
            LHSNode *node = 0;
            if (0 == deviceLogic.GetStringLength())
                node = &(payload->mLHS->makeNode("and"));
            else
                node = &(payload->mLHS->makeNode(deviceLogic.GetString()));

            for (size_t i = 0; i < deviceStatus.Size(); ++i) {
                rapidjson::Value &dev = deviceStatus[i];
                if (!dev.IsObject()) {
                    LOGE("parse conditions.deviceCondition.deviceStatus[%d] error!\n", i);
                    return false;
                }
                rapidjson::Value &did = dev["deviceId"];
                if (!did.IsString()) {
                    LOGE("parse conditions.deviceCondition.deviceStatus[%d].deviceId error!\n", i);
                    return false;
                }
                rapidjson::Value &pro = dev["propName"];
                if (!pro.IsString()) {
                    LOGE("parse conditions.deviceCondition.deviceStatus[%d].propName error!\n", i);
                    return false;
                }
                rapidjson::Value &val = dev["propValue"];
                if (!val.IsString()) {
                    LOGE("parse conditions.deviceCondition.deviceStatus[%d].proValue error!\n", i);
                    return false;
                }
                Condition &insCond = node->makeCond(CT_INSTANCE,
                    getClassNameByDeviceId(did.GetString()), did.GetString());

                SlotPoint &slotpoint = insCond.makeSlot(pro.GetString());
                char *propval = (char*)val.GetString();
                char *t_and = strstr(propval, "and");
                char *t_or = strstr(propval, "or");

                if (!t_and && !t_or) {
                    if (!parsePropValue(propval, slotpoint)) {
                        LOGE("parse conditions.deviceCondition.deviceStatus[%d] error!\n", i);
                        return false;
                    }
                }

                char lside[64] = { 0 };
                char rside[64] = { 0 };
                /* and */
                if (t_and) {
                    slotpoint.mCellLogic = "&";
                    strncpy(lside, propval, t_and - propval);
                    strcpy(rside, t_and+3);
                }

                /* or */
                if (t_or) {
                    slotpoint.mCellLogic = "|";
                    strncpy(lside, propval, t_or - propval);
                    strcpy(rside, t_or+2);
                }

                if (lside[0] && rside[0]) {
                    if (!parsePropValue(lside, slotpoint)) {
                        LOGE("parse conditions.deviceCondition.deviceStatus[%d] error!\n", i);
                        return false;
                    }
                    if (!parsePropValue(rside, slotpoint)) {
                        LOGE("parse conditions.deviceCondition.deviceStatus[%d] error!\n", i);
                        return false;
                    }
                }
            } /* end for */
        } /* end if IsObject */
    }/*}}}*/

    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parseActions(rapidjson::Value &actions, std::shared_ptr<RulePayload> payload)
{/*{{{*/
    if (payload->mNotifyEnable && actions.HasMember("notify")) {/*{{{*/
        rapidjson::Value &notify = actions["notify"];
        if (notify.IsObject() && !notify.ObjectEmpty()) {
            rapidjson::Value &title = notify["title"];
            rapidjson::Value &message = notify["message"];
            if (!title.IsString() || !message.IsString()) {
                LOGE("parse actions.notify error!\n");
                return false;
            }
            char id[9] = { 0 };
            sprintf(id, "n-%06u", rand() % 1000000);
            payload->mRHS->makeAction(AT_NOTIFY, id, title.GetString(), message.GetString());
        }
    }/*}}}*/

    if (actions.HasMember("deviceControl")) {/*{{{*/
        rapidjson::Value &deviceControl = actions["deviceControl"];
        if (deviceControl.IsArray() && deviceControl.Size() > 0) {
            for (size_t i = 0; i < deviceControl.Size(); ++i) {
                rapidjson::Value &dev = deviceControl[i];
                if (!dev.IsObject()) {
                    LOGE("parse actions.deviceControl[%d] error!\n", i);
                    return false;
                }
                rapidjson::Value &deviceId = dev["deviceId"];
                rapidjson::Value &propName = dev["propName"];
                rapidjson::Value &propValue = dev["propValue"];
                if (!deviceId.IsString() || !propName.IsString() || !propValue.IsString()) {\
                    LOGE("parse actions.deviceControl[%d].xxx error!\n", i);
                    return false;
                }
                std::string val = propValue.GetString();
                if (strstr(val.c_str(), "true"))
                    val = "1";
                else if (strstr(val.c_str(), "false"))
                    val = "0";
                payload->mRHS->makeAction(AT_CONTROL,
                    deviceId.GetString(),
                    propName.GetString(), val);
            }
        }
    }/*}}}*/

    if (actions.HasMember("execScene")) {/*{{{*/
        rapidjson::Value &scene = actions["execScene"];
        if (scene.IsString() && scene.GetStringLength() > 1) {
            std::string assert;
            assert.append("(switch-scene ").append(" default ").append(scene.GetString()).append(")");
            payload->mRHS->makeAction(AT_ASSERT, assert);
        }
    }/*}}}*/

    if (actions.HasMember("manualRuleId")) {/*{{{*/
        rapidjson::Value &manualRuleId = actions["manualRuleId"];
        if (manualRuleId.IsArray() && manualRuleId.Size() > 0) {
            for (size_t i = 0; i < manualRuleId.Size(); ++i) {
                rapidjson::Value &rul =  manualRuleId[i];
                if (!rul.IsString()) {
                    LOGE("parse manualSceneId[%d] error!\n", i);
                    return false;
                }
                payload->mRHS->makeAction(AT_SCENE, rul.GetString());
            }
        }
    }/*}}}*/
    return true;
}/*}}}*/

bool ElinkCloudDataChannel::parsePropValue(const char *cpropval, Slot &slot)
{/*{{{*/
    char *propval = (char*)cpropval;
    char *t_and = strstr(propval, "and");
    if (!t_and)
        return true;
    char lside[64] = { 0 };
    char rside[64] = { 0 };
    strncpy(lside, propval, t_and - propval);
    strcpy(rside, t_and+3);

    if (lside[0]) {
        char *egt = strstr(lside, ">=");
        if (!egt)
            return false;
        slot.mMin = std::string(egt + 2);
        stringTrim(slot.mMin);
    }

    if (rside[0]) {
        char *elt = strstr(propval, "<=");
        if (!elt)
            return false;
        slot.mMax = std::string(elt + 2);
        stringTrim(slot.mMax);
    }
    return true;
}/*}}}*/
} /* namespace HB */
