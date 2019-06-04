/***************************************************************************
 *  GatewaysAPI.cpp -
 *
 *  Created: 2019-02-16 15:43:50
 *
 *  Copyright QRS
 ****************************************************************************/

#include "GatewaysAPI.h"
#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"
#include "HBHelper.h"
#include "HBDatabase.h"
#include "GatewayConnectTable.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <fstream>
#include <sstream>
#include <map>

using namespace rapidjson;
// using namespace UTILS;
// using namespace OIC::Service::HB;

namespace HB {

static std::map<int, const char*> gErrorCodes {/*{{{*/
    {-1000, "gateway api common error"},
    {-1001, "check token error"},
    {-1002, "document parse error"},

    {-1101, "gateway add inner error"},
    {-1102, "gateway add inner error, no ip"},
    {-1201, "gateway remove inner error"},
    {-1202, "gateway remove inner error, not found gid from db"},
    {-1301, "gateway status inner error"},
    {-1401, "gateway net inner error"},
};/*}}}*/

class GatewayStatusResult {/*{{{*/
public:
    GatewayStatusResult(const crow::request& req, crow::response& res)
        : mReq(req), mRes(res), mCode(0), mResext("") {}

    ~GatewayStatusResult();

    void setStatusCode(int code) { mCode = code; }
    void setStatusCode(int code, const std::string &ext) {
        mCode = code;
        mResext = ext;
    }
    bool checkToken() { return true; }

private:
    const crow::request& mReq;
    crow::response& mRes;
    int mCode;
    std::string mResext;
};/*}}}*/

GatewayStatusResult::~GatewayStatusResult()
{/*{{{*/
    std::stringstream ss;

    /* TODO */
    ss << "{\"requestId\":\"abcddcba\", \"request\":\"" << mReq.url << "\", \"status\":";

    if (mCode == 0)
        ss << "1";
    else
        ss << "0, \"errors\": {\"message\":\"" << gErrorCodes[mCode] << "\", \"code\":\"" << mCode << "\"}" ;
    if (!mResext.empty())
        ss << ", " << mResext;
    ss << "}";

    mRes.write(ss.str().c_str());
    mRes.end();
}/*}}}*/

static void _add(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    int ret = -1;
    Document doc;
    doc.Parse(req.body.c_str());

    HH_LOGD("body[%s]\n", req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& gatewayType = doc["gatewayType"];
    if (!gatewayType.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& gatewayId = doc["gatewayId"];
    if (!gatewayId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& accessKey = doc["accessKey"];
    if (!accessKey.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    std::string ip("0.0.0.0");
    if (doc.HasMember("gatewayIp")) {
        Value& gatewayIp = doc["gatewayIp"];
        if (!gatewayIp.IsString()) {
            status.setStatusCode(-1002);
            return;
        }
        ip = gatewayIp.GetString();
    }

    GatewayTableInfo info;
    info.nGid = gatewayId.GetString();
    info.nDid = gatewayId.GetString();
    info.nType = gatewayType.GetString();
    info.nKey = accessKey.GetString();
    info.nIp = ip;
    info.nEnable = 1;
    mainDB().updateOrInsert(info);

    ret = startBridge(info.nType.c_str(), info.nGid.c_str(), info.nKey.c_str(), info.nIp.c_str());
    std::string result;
    result.append("\"result\":");
    result.append("{\"ret\":").append("\"").append(ret < 0 ? "fail" : "success").append("\"}");
    status.setStatusCode(0, result);
}/*}}}*/

static void _delete(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& gatewayId = doc["gatewayId"];
    if (!gatewayId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    std::string gid = gatewayId.GetString();

    killBridge(gid.c_str());

    std::vector<GatewayTableInfo> infos;
    mainDB().queryBy(infos, gid);
    if (infos.size() != 1) {
        status.setStatusCode(-1202);
        return;
    }
    mainDB().deleteBy(infos[0]);

    std::string result;
    result.append("\"result\":{\"ret\":").append("\"").append("success").append("\"}");
    status.setStatusCode(0, result);
    return;
}/*}}}*/

static void _status(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& gatewayId = doc["gatewayId"];
    if (!gatewayId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }
    HBGatewayStatus st;
    int ret = deviceManager().GetGatewayStatus(gatewayId.GetString(), st);
    if (0 != ret) {
        status.setStatusCode(-1301);
        return;
    }
    std::string val("0");
    switch (st) {
        case HB_GATEWAY_STATUS_CONNECTED:
            val.assign("1");
            break;
        case HB_GATEWAY_STATUS_READY:
            val.assign("2");
            break;
    }
    std::string result;
    result.append("\"result\":{\"status\":").append("\"").append(val).append("\"}");
    status.setStatusCode(0, result);
}/*}}}*/

static void _list(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    std::vector<GatewayTableInfo> infos;
    mainDB().queryBy(infos, GatewayTableInfo());
    std::string result("\"result\":[");
    for (size_t i = 0; i < infos.size(); ++i) {
        if (i) result.append(",");
        result.append("{\"ownedStatus\":").append("\"").append("HB_binded").append("\",");
        result.append("\"gatewayType\":").append("\"").append(infos[i].nType).append("\",");
        result.append("\"gatewayId\":").append("\"").append(infos[i].nGid).append("\",");
        result.append("\"accessKey\":").append("\"").append(infos[i].nKey).append("\"}");
    }
    result.append("]");
    status.setStatusCode(0, result);
    return;
}/*}}}*/

static void _net(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& gatewayId = doc["gatewayId"];
    if (!gatewayId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    int ret = deviceManager().EnableGatewayNet(gatewayId.GetString());
    if (0 != ret) {
        status.setStatusCode(-1401);
        return;
    }
    std::string result("\"result\":{");
    result.append("\"ret\":").append("\"").append("success").append("\",");
    result.append("\"maxDuration\":").append("\"5\"").append("}");
    status.setStatusCode(0, result);
}/*}}}*/

extern void initGatewaysAPI(APP& app)
{
    CROW_ROUTE(app, "/api/gateway/add").methods(REST_POST)(_add);
    CROW_ROUTE(app, "/api/gateway/remove").methods(REST_POST)(_delete);
    CROW_ROUTE(app, "/api/gateway/status").methods(REST_POST)(_status);
    CROW_ROUTE(app, "/api/gateway/list").methods(REST_POST)(_list);
    CROW_ROUTE(app, "/api/gateway/net").methods(REST_POST)(_net);
}

} /* namespace HB */
