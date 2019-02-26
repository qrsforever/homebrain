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

static std::string gTest("\"result\":[]");

namespace HB {

static std::map<int, const char*> gErrorCodes {/*{{{*/
    {-1000, "gateway api common error"},
    {-1001, "check token error"},
    {-1002, "document parse error"},

    {-1101, "gateway add inner error"},
    {-1102, "gateway add inner error, no ip"},
    {-1103, "gateway add inner error, write config file"},
    {-1201, "gateway remove inner error"},
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

    Document doc;
    doc.Parse(req.body.c_str());

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
        ip = gatewayId.GetString();
    }

    if (!strncmp(gatewayType.GetString(), "hue", 3)) {
        if (ip == "0.0.0.0") {
            status.setStatusCode(-1102);
            return;
        }
        std::string path("hue_bridge.conf");
        std::ofstream of(path);
        if (!of.is_open()) {
            status.setStatusCode(-1103);
            return;
        }

        of << "{\"id\":\"" << gatewayId.GetString() << "\", \"username\":\"" << accessKey.GetString() << "\",";
        of << "\"internalipaddress\":\"" << ip << "\"}" << std::endl;
        of.close();
    }

    // int ret = startBridge(info.nType.c_str(), info.nGid, accessKey.GetString());
    std::string result;
    // result.append("\"result\":");
    // result.append("{\"ret\":").append("\"").append(ret < 0 ? "fail" : "success").append("\"}");
    status.setStatusCode(0, result);
}/*}}}*/

static void _remove(const crow::request& req, crow::response& res)
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

    killBridge(gatewayId.GetString());

    gTest.assign("\"result\":[]");
    std::string result;
    result.append("\"result\":{\"ret\":").append("\"").append("success").append("\"}");
    status.setStatusCode(0, result);
    return;
}/*}}}*/

static void _status(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    // gTest.assign("\"result\":[{");
    // gTest.append("\"ownedStatus\":").append("\"").append("HB_binded").append("\",");
    // gTest.append("\"gatewayType\":").append("\"").append(gatewayType.GetString()).append("\",");
    // gTest.append("\"gatewayIp\":").append("\"").append(ip).append("\",");
    // gTest.append("\"gatewayId\":").append("\"").append().append("\",");
    // gTest.append("\"accessKey\":").append("\"").append(accessKey.GetString()).append("\"}]");
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
    // HBGatewayStatus status;
    // int ret = deviceManager().GetGatewayStatus(gatewayId.GetString(), status);
    // if (0 != ret) {
    //     status.setStatusCode(-1301);
    //     return;
    // }
    static int test = 0;
    std::string result;
    if (++test % 4 == 0)
        result.append("\"result\":{\"status\":").append("\"").append("2").append("\"}");
    else
        result.append("\"result\":{\"status\":").append("\"").append("1").append("\"}");
    status.setStatusCode(0, result);
}/*}}}*/

static void _list(const crow::request& req, crow::response& res)
{/*{{{*/
    GatewayStatusResult status(req, res);

    // std::string result("\"result\":[{");
    // result.append("\"ownedStatus\":").append("\"").append("HB_binded").append("\",");
    // result.append("\"gatewayType\":").append("\"").append("konke").append("\",");
    // result.append("\"gatewayId\":").append("\"").append("02169").append("\",");
    // result.append("\"accessKey\":").append("\"").append("C3E2C6C75DCC222A1472E67B57F05B83").append("\"}]");
    status.setStatusCode(0, gTest);
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

    // int ret = deviceManager().EnableGatewayNet(gatewayId.GetString());
    // if (0 != ret) {
    //     status.setStatusCode(-1401);
    //     return;
    // }
    std::string result("\"result\":{");
    result.append("\"ret\":").append("\"").append("success").append("\",");
    result.append("\"maxDuration\":").append("\"5\"").append("}");
    status.setStatusCode(0, result);
}/*}}}*/

extern void initGatewaysAPI(APP& app)
{
    CROW_ROUTE(app, "/api/gateway/add").methods(REST_POST)(_add);
    CROW_ROUTE(app, "/api/gateway/remove").methods(REST_POST)(_remove);
    CROW_ROUTE(app, "/api/gateway/status").methods(REST_POST)(_status);
    CROW_ROUTE(app, "/api/gateway/list").methods(REST_POST)(_list);
    CROW_ROUTE(app, "/api/gateway/net").methods(REST_POST)(_net);
}

} /* namespace HB */
