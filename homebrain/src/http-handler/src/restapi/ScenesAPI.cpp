/***************************************************************************
 *  ScenesAPI.cpp -
 *
 *  Created: 2019-06-05 17:44:15
 *
 *  Copyright QRS
 ****************************************************************************/

#include "ScenesAPI.h"
#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"
#include "HBDatabase.h"
#include "Common.h"
#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "StringArray.h"
#include "StringData.h"
#include "CloudDataChannel.h"
#include "SceneEngineTable.h"

#include "rapidjson/document.h"

#include <sstream>

using namespace UTILS;
using namespace rapidjson;

#define SCRIPT_SYNC

namespace HB {

enum SceneErrorCode {
    RC_FAIL = -1,
    RC_SUCCESS = 0,
    RC_TOKEN_EMPTY = 2,
    RC_TOKEN_INVALID = 3,
};

static std::map<std::string, const char*> gUriDecr {
    {"/api/familyscene/listscene",    "场景列表"},
    {"/api/familyscene/deletescene",  "场景删除"},
    {"/api/familyscene/modifyscene",  "场景修改"},
    {"/api/familyscene/queryscene",   "场景查询"},
    {"/api/familyscene/executescene", "场景执行"}
};

class SceneStatusResult {/*{{{*/
public:
    SceneStatusResult(const crow::request& req, crow::response& res)
        : mReq(req), mRes(res), mCode(0), mResext("") {}

    ~SceneStatusResult();

    void setStatusCode(int code) { mCode = code; }
    void setStatusCode(int code, const std::string &ext) {
        mCode = code;
        mResext = ext;
    }

private:
    const crow::request& mReq;
    crow::response& mRes;
    int mCode;
    std::string mResext;
};/*}}}*/

SceneStatusResult::~SceneStatusResult()
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

static void _list(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    SceneStatusResult status(req, res);

    std::vector<SceneTableInfo> infos;
    mainDB().queryBy(infos, SceneTableInfo());

    std::string extstr;
    extstr.append("\"data\":[");
    for (unsigned int i = 0; i < infos.size(); ++i) {
        if (i) extstr.append(",");
        extstr.append("{\"sceneName\":\"").append(infos[i].nSceneName).append("\"");
        extstr.append(",\"sceneId\":\"").append(infos[i].nSceneId).append("\"}");
    }
    extstr.append("]");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _delete(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    SceneStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    HH_LOGD("body: %s\n", req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& sceneId = doc["sceneId"];
    if (!sceneId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    std::vector<SceneTableInfo> infos;
    mainDB().queryBy(infos, sceneId.GetString());
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"error: not found the scene!\"");
        return;
    }
    ruleEngine().disableRule(infos[0].nSceneId);

    mainDB().deleteBy(infos[0]);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(infos[0].nSceneId).append("\"");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _modify(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    SceneStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    HH_LOGD("body: %s\n", req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    std::shared_ptr<ScenePayload> payload = std::make_shared<ScenePayload>();
    if (!ElinkCloudDataChannel::parseScene(doc, payload)) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse rule error!\"");
        return;
    }

    SceneTableInfo info;
    info.nSceneId = payload->mSid;
    info.nSceneName = payload->mName;
    info.nJsonData = req.body;
    info.nScriptData = std::move(payload->toString());

    mainDB().updateOrInsert(info);

    ruleEngine().enableRule(info.nSceneId, info.nScriptData, true);

    std::string extstr;
    extstr.append("\"sceneId\":\"").append(info.nSceneId).append("\"");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _query(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    SceneStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& sceneId = doc["sceneId"];
    if (!sceneId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse ruleid error!\"");
        return;
    }

    std::vector<SceneTableInfo> infos;
    mainDB().queryBy(infos, sceneId.GetString());
    std::string extstr;
    if (infos.size() == 1)
        extstr.append("\"scene\":").append(infos[0].nJsonData);
    else
        extstr.append("\"scene\":{}");
    status.setStatusCode(RC_SUCCESS, extstr);
    return;
}/*}}}*/

static void _execute(const crow::request& req, crow::response& res)
{/*{{{*/
    HH_LOGD("api:%s\n", req.url.c_str());

    SceneStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse error!\"");
        return;
    }

    Value& sceneId = doc["sceneId"];
    if (!sceneId.IsString()) {
        status.setStatusCode(RC_FAIL, "\"details\": \"doc parse sceneId error!\"");
        return;
    }

    std::vector<SceneTableInfo> infos;
    mainDB().queryBy(infos, sceneId.GetString());
    if (infos.size() != 1) {
        status.setStatusCode(RC_FAIL, "\"details\": \"not found sceneId!\"");
        return;
    }

    std::string defroom("default");
    Value& room = doc["room"];
    if (room.IsString())
        defroom.assign(room.GetString());

    ruleEngine().executeScene(infos[0].nSceneName, defroom, true);
    std::string extstr;
    extstr.append("\"sceneId\":\"").append(infos[0].nSceneId).append("\"");
    status.setStatusCode(RC_SUCCESS, extstr);
}/*}}}*/

extern void initScenesAPI(APP& app)
{
    CROW_ROUTE(app, "/api/familyscene/listscene").methods(REST_POST)(_list);
    CROW_ROUTE(app, "/api/familyscene/deletescene").methods(REST_POST)(_delete);
    CROW_ROUTE(app, "/api/familyscene/modifyscene").methods(REST_POST)(_modify);
    CROW_ROUTE(app, "/api/familyscene/queryscene").methods(REST_POST)(_query);
    CROW_ROUTE(app, "/api/familyscene/executescene").methods(REST_POST)(_execute);
}

} /* namespace HB */
