/***************************************************************************
 *  RulesAPI.cpp -
 *
 *  Created: 2018-10-25 20:40:29
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RulesAPI.h"
#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"
#include "HBDatabase.h"
#include "Common.h"
#include "StringArray.h"
#include "StringData.h"
#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "CloudDataChannel.h"
#include "RuleEngineTable.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <sstream>

using namespace UTILS;
using namespace rapidjson;

namespace HB {

enum RuleErrorCode {
    RC_FAIL = -1,
    RC_SUCCESS = 0,
    RC_TOKEN_EMPTY = 2,
    RC_TOKEN_INVALID = 3,
};

static std::map<std::string, const char*> gUriDecr {
    {"/api/familyscene/add",     "规则添加"},
    {"/api/familyscene/delete",  "规则删除"},
    {"/api/familyscene/modify",  "规则修改"},
    {"/api/familyscene/query",   "规则查询"},
    {"/api/familyscene/listall", "规则列表查询"},
    {"/api/familyscene/execute", "规则执行"},
    {"/api/familyscene/enable",  "规则使能"}
};

class RuleStatusResult {/*{{{*/
public:
    RuleStatusResult(const crow::request& req, crow::response& res)
        : mReq(req), mRes(res), mCode(0), mResext("") {}

    ~RuleStatusResult();

    void setStatusCode(int code) { mCode = code; }
    void setStatusCode(int code, const std::string &ext) {
        mCode = code;
        mResext = ext;
    }
    bool checkToken();

private:
    const crow::request& mReq;
    crow::response& mRes;
    int mCode;
    std::string mResext;
};/*}}}*/

RuleStatusResult::~RuleStatusResult()
{/*{{{*/
    /* reponse result to client endpoint */
    std::stringstream ss;

    /* TODO */
    ss << "{\"requestId\":\"abcddcba\", \"request\":\"" << mReq.url << "\", \"result\":{\"resInfo\":";

    if (mCode == RC_SUCCESS) {
        ss << "\"" << gUriDecr[mReq.url] << "成功\"";
        if (!mResext.empty())
            ss << "," << mResext;
        ss << "}, \"status\": 1}";
    } else {
        ss << "\"" << gUriDecr[mReq.url] << "失败\"";
        if (!mResext.empty())
            ss << "," << mResext;
        ss << "}, \"status\":" << mCode << "}";
    }
    mRes.write(ss.str().c_str());
    mRes.end();
}/*}}}*/

bool RuleStatusResult::checkToken()
{/*{{{*/
    return true;
    // std::string token = mReq.get_header_value("token");
    // int ret = HB::checkToken(token);
    // if (ret != 0) {
    //     mCode = RC_TOKEN_INVALID;
    //     return false;
    // }
    // return true;
}/*}}}*/

static void _add(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    /* random generate sceneId number */
    std::string genId = genRandom();
    Value& ruleId = doc["sceneId"];
    ruleId.SetString(genId.c_str(), genId.size());

    /* parse rule json which is samle schema with elink */
    std::shared_ptr<RulePayload> payload = std::make_shared<RulePayload>();
    if (!ElinkCloudDataChannel::parseRule(doc, payload)) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse rule error!\"");
        return;
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    /* update database */
    RuleTableInfo info;
    info.nRuleId = genId;
    info.nRuleName = payload->mRuleName;
    info.nRuleDesr = payload->mRuleDesr;
    info.nRuleVer = payload->mVersion;
    info.nEnable = payload->mEnable ? 1 : 0;
    info.nManual = payload->mManual ? 1 : 0;
    info.nJsonData = buffer.GetString();
    info.nScriptData = std::move(payload->toString());

    /* check actions */
    if (!payload->mManual) {
        // only auto rule can have manual action
        std::vector<std::string> &friends = payload->mRHS->mSceneIds;
        if (friends.size() > 0) {
            for (size_t i = 0; i < friends.size(); ++i) {
                std::vector<RuleTableInfo> tmpinfos;
                mainDB().queryBy(tmpinfos, friends[i]);
                // only manual rule can enter
                if (tmpinfos.size() == 1 && tmpinfos[0].nManual) {
                    if (!tmpinfos[0].nFriends.empty())
                        tmpinfos[0].nFriends.append("|");
                    tmpinfos[0].nFriends.append(genId);
                    mainDB().updateOrInsert(tmpinfos[0]);
                    if (!info.nFriends.empty())
                        info.nFriends.append("|");
                    info.nFriends.append(friends[i]);
                }
            }
        }
    }
    mainDB().updateOrInsert(info);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(info.nRuleId).append("\"");
    /* enable rule */
    if (info.nEnable) {
        bool result = ruleEngine().enableRule(info.nRuleId, info.nScriptData, true);
        extstr.append(",\"ruleScript\":").append(result ? "0" : "-1");
    }
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _modify(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    /* parse rule json which is samle schema with elink */
    std::shared_ptr<RulePayload> payload = std::make_shared<RulePayload>();
    if (!ElinkCloudDataChannel::parseRule(doc, payload)) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse rule error!\"");
        return;
    }

    /* update database */
    std::vector<RuleTableInfo> infos;
    mainDB().queryBy(infos, payload->mRuleID);
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"not found the rule error!\"");
        return;
    }

    RuleTableInfo &info = infos[0];
    if (payload->mManual && !info.nManual && !info.nFriends.empty()) {
        // auto to manual
        std::vector<std::string> friends = stringSplit(info.nFriends, "|");
        info.nFriends.assign("");
        for (size_t i = 0, l = friends.size(); i < l; ++i) {
            std::vector<RuleTableInfo> tmpinfos;
            mainDB().queryBy(tmpinfos, friends[i]);
            if (tmpinfos.size() == 1 && !tmpinfos[0].nFriends.empty()) {
                std::vector<std::string> tmpfriends = stringSplit(tmpinfos[0].nFriends, "|");
                tmpinfos[0].nFriends.assign("");
                for (size_t j = 0, s = tmpfriends.size(); j < s; ++j) {
                    if (tmpfriends[j] == info.nRuleId)
                        continue;
                    if (!tmpinfos[0].nFriends.empty())
                        tmpinfos[0].nFriends.append("|");
                    tmpinfos[0].nFriends.append(tmpfriends[j]);
                }
                mainDB().updateOrInsert(tmpinfos[0]);
            }
        }
    }
    if (!payload->mManual && info.nManual && !info.nFriends.empty()) {
        // manual with friends to auto
        status.setStatusCode(RC_FAIL, "\"details\": \"not allow modify rule when manual rule has friends!\"");
        return;
    }
    info.nRuleName = payload->mRuleName;
    info.nRuleDesr = payload->mRuleDesr;
    info.nRuleVer = payload->mVersion;
    info.nEnable = payload->mEnable ? 1 : 0;
    info.nManual = payload->mManual ? 1 : 0;
    info.nJsonData = req.body;
    info.nScriptData = std::move(payload->toString());

    HH_LOGD("[%s]'s friends:%s\n", info.nRuleId.c_str(), info.nFriends.c_str());

    if (!payload->mManual) {
        // only auto rule can have manual action
        // delete when ui uncheck, check the diff before change and after change
        std::vector<std::string> oldfriends = stringSplit(info.nFriends, "|");
        std::vector<std::string> &friends = payload->mRHS->mSceneIds;
        for (size_t i = 0; i < oldfriends.size(); ++i) {
            bool find = false;
            for (size_t j = 0; j < friends.size(); ++j) {
                if (oldfriends[i] == friends[i]) {
                    find = true;
                    break;
                }
            }
            if (!find) {
                std::vector<RuleTableInfo> tmpinfos;
                mainDB().queryBy(tmpinfos, oldfriends[i]);
                if (tmpinfos.size() == 1 && !tmpinfos[0].nFriends.empty()) {
                    std::vector<std::string> tmpfriends = stringSplit(tmpinfos[0].nFriends, "|");
                    tmpinfos[0].nFriends.assign("");
                    for (size_t j = 0, s = tmpfriends.size(); j < s; ++j) {
                        // filter out the uncheck rules
                        if (tmpfriends[j] == info.nRuleId)
                            continue;
                        if (!tmpinfos[0].nFriends.empty())
                            tmpinfos[0].nFriends.append("|");
                        tmpinfos[0].nFriends.append(tmpfriends[j]);
                    }
                    mainDB().updateOrInsert(tmpinfos[0]);
                }
            }
        }

        info.nFriends.assign("");
        for (size_t i = 0; i < friends.size(); ++i) {
            std::vector<RuleTableInfo> tmpinfos;
            mainDB().queryBy(tmpinfos, friends[i]);
            // only manual rule can enter
            if (tmpinfos.size() == 1 && tmpinfos[0].nManual) {
                if (tmpinfos[0].nFriends.find(info.nRuleId) == std::string::npos) {
                    if (!tmpinfos[0].nFriends.empty())
                        tmpinfos[0].nFriends.append("|");
                    tmpinfos[0].nFriends.append(info.nRuleId);
                    mainDB().updateOrInsert(tmpinfos[0]);
                }
                if (!info.nFriends.empty())
                    info.nFriends.append("|");
                info.nFriends.append(friends[i]);
            }
        }
        HH_LOGD("[%s]'s friends:%s\n", info.nRuleId.c_str(), info.nFriends.c_str());
    }
    mainDB().updateOrInsert(info);

    bool result = false;
    std::string extstr;
    extstr.append("\"sceneId\":\"").append(info.nRuleId).append("\"");
    if (info.nEnable)
        result = ruleEngine().enableRule(info.nRuleId, info.nScriptData, true);
    else
        result = ruleEngine().disableRule(info.nRuleId, true);
    extstr.append(",\"ruleScript\":").append(result ? "0" : "-1");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _delete(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& ruleId = doc["sceneId"];
    if (!ruleId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    std::vector<RuleTableInfo> infos;
    mainDB().queryBy(infos, ruleId.GetString());
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"error: not found the rule!\"");
        return;
    }
    RuleTableInfo &info = infos[0];

    if (info.nManual && !info.nFriends.empty()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"error: this rule have associated rules!\"");
        return;
    }

    if (!info.nManual && !info.nFriends.empty()) {
        std::vector<std::string> friends = stringSplit(info.nFriends, "|");
        for (size_t i = 0, l = friends.size(); i < l; ++i) {
            std::vector<RuleTableInfo> tmpinfos;
            mainDB().queryBy(tmpinfos, friends[i]);
            if (tmpinfos.size() == 1 && !tmpinfos[0].nFriends.empty()) {
                bool flag = false;
                std::vector<std::string> tmpfriends = stringSplit(tmpinfos[0].nFriends, "|");
                tmpinfos[0].nFriends.assign("");
                for (size_t j = 0, s = tmpfriends.size(); j < s; ++j) {
                    if (tmpfriends[j] == info.nRuleId)
                        continue;
                    if (!tmpinfos[0].nFriends.empty())
                        tmpinfos[0].nFriends.append("|");
                    tmpinfos[0].nFriends.append(tmpfriends[j]);
                    if (!flag) flag = true;
                }
                if (flag)
                    mainDB().updateOrInsert(tmpinfos[0]);
            }
        }
    }
    mainDB().deleteBy(info);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(infos[0].nRuleId).append("\"");
    bool result = ruleEngine().disableRule(infos[0].nRuleId, true);
    extstr.append(",\"ruleScript\":").append(result ? "0" : "-1");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _query(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& ruleId = doc["sceneId"];
    if (!ruleId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    std::vector<RuleTableInfo> infos;
    mainDB().queryBy(infos, ruleId.GetString());
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"db query error!\"");
        return;
    }

    std::string extstr;
    extstr.append("\"scene\":").append(infos[0].nJsonData);
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _listall(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    std::vector<RuleTableInfo> rules;
    mainDB().queryBy(rules, RuleTableInfo());

    std::string extstr;
    extstr.append("\"data\":[");
    for (unsigned int i = 0, l = rules.size(); i < l; ++i) {
        if (i) extstr.append(",");
        /* TODO */
        extstr.append("{\"triggerEnabled\":\"").append(int2String(rules[i].nEnable)).append("\"");
        extstr.append(",\"manualEnabled\":\"").append(int2String(rules[i].nManual)).append("\"");
        extstr.append(",\"sceneName\":\"").append(rules[i].nRuleName).append("\"");
        extstr.append(",\"sceneId\":\"").append(rules[i].nRuleId).append("\"");
        extstr.append(",\"friends\":\"").append(rules[i].nFriends).append("\"");
        extstr.append(",\"description\":\"").append(rules[i].nRuleDesr).append("\"}");
    }
    extstr.append("]");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _execute(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);
    if (!status.checkToken())
        return;

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& ruleId = doc["sceneId"];
    if (!ruleId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }
    ruleEngine().triggerRule(ruleId.GetString(), true);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(ruleId.GetString()).append("\"");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _enable(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    RuleStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& ruleId = doc["sceneId"];
    if (!ruleId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    Value& enable = doc["enable"];
    if (!enable.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    std::vector<RuleTableInfo> infos;
    mainDB().queryBy(infos, ruleId.GetString());
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"not found the rule error!\"");
        return;
    }

    Document jsonData;
    jsonData.Parse(infos[0].nJsonData.c_str());
    Value &enabled = jsonData["trigger"]["switch"]["triggerEnabled"];

    infos[0].nEnable = atoi(enable.GetString());

    if (infos[0].nEnable) {
        enabled.SetString("on", 2);
        ruleEngine().enableRule(infos[0].nRuleId, infos[0].nScriptData, true);
    } else {
        enabled.SetString("off", 3);
        ruleEngine().disableRule(infos[0].nRuleId, true);
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    jsonData.Accept(writer);

    infos[0].nJsonData = buffer.GetString();

    mainDB().updateOrInsert(infos[0]);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(infos[0].nRuleId).append("\"");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

extern void initRulesAPI(APP& app)
{
    CROW_ROUTE(app, "/api/familyscene/add").methods(REST_POST)(_add);
    CROW_ROUTE(app, "/api/familyscene/delete").methods(REST_POST)(_delete);
    CROW_ROUTE(app, "/api/familyscene/modify").methods(REST_POST)(_modify);
    CROW_ROUTE(app, "/api/familyscene/query").methods(REST_POST)(_query);
    CROW_ROUTE(app, "/api/familyscene/listall").methods(REST_POST)(_listall);
    CROW_ROUTE(app, "/api/familyscene/execute").methods(REST_POST)(_execute);
    CROW_ROUTE(app, "/api/familyscene/enable").methods(REST_POST)(_enable);
}

} /* namespace HB */
