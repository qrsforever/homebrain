/***************************************************************************
 *  DevicesAPI.cpp -
 *
 *  Created: 2018-10-25 19:19:48
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DevicesAPI.h"
#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"
#include "DeviceProfileTable.h"
#include "HBHelper.h"
#include "HBDatabase.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <sstream>
#include <map>

using namespace rapidjson;
using namespace UTILS;
using namespace OIC::Service::HB;

namespace HB {

static std::map<int, const char*> gErrorCodes {/*{{{*/
    {-1000, "device api common error"},
    {-1001, "check token error"},
    {-1002, "document parse error"},


    {-1100, "discovery error"},
    {-1101, "discovery inner error"},


    {-1200, "list error"},

    {-1300, "bind error"},
    {-1301, "bind inner error"},

    {-1400, "isonline error"},

    {-1500, "profile error"},
    {-1501, "profile inner error"},

    {-1600, "status error"},
    {-1601, "status inner1 error"},
    {-1602, "status inner2 error"},

    {-1700, "operate error"},
    {-1701, "operate inner error"},
};/*}}}*/

class DeviceStatusResult {/*{{{*/
public:
    DeviceStatusResult(const crow::request& req, crow::response& res)
        : mReq(req), mRes(res), mCode(0), mResext("") {}

    ~DeviceStatusResult();

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

DeviceStatusResult::~DeviceStatusResult()
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

static void _discovery(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    int ret = deviceManager().DiscoverDevices();

    if (0 == ret)
        status.setStatusCode(0, "\"result\":{\"ret\": \"success\", \"maxDuration\": \"5\"}");
    else
        status.setStatusCode(-1100);
    return ;
}/*}}}*/

static void _list(const crow::request& req, crow::response& res)
{/*{{{*/

    DeviceStatusResult status(req, res);

    std::map<std::string, std::string> deviceList;
    int ret = deviceManager().GetDeviceList(deviceList);
    if (0 != ret) {
        status.setStatusCode(-1200);
        return ;
    }
    std::string result("\"result\":[");
    std::string ownedStr;
    HBDeviceOwnedStatus ownedStatus;
    std::vector<DeviceTableInfo> prfs;
    DeviceTableInfo filter;
    bool firstItem = true;
    for (auto it = deviceList.begin(); it != deviceList.end(); ++it) {
        if (it->second == "oic.d.bridge")
            continue;
        if (firstItem) firstItem = false; else result.append(",");
        ownedStatus = HB_DEVICE_UNBINDED;
        deviceManager().GetDeviceOwnedStatus(it->first, ownedStatus);
        if (ownedStatus == HB_DEVICE_UNBINDED)
            ownedStr = "unbinded";
        else if (ownedStatus == HB_DEVICE_LOCAL_BINDED)
            ownedStr = "HB_binded";
        else if (ownedStatus == HB_DEVICE_CLOUD_BINDED)
            ownedStr = "cloud_binded";
        filter.nDeviceType = it->second;
        mainDB().queryBy(prfs, filter);
        if (prfs.size() == 1) {
            result.append("{\"deviceId\":\"").append(it->first).append("\"");
            result.append(",\"typeId\":\"").append(it->second).append("\"");
            result.append(",\"label\":\"").append(prfs[0].nDeviceName).append("\"");
            result.append(",\"iconid\":\"").append(prfs[0].nIconId).append("\"");
            result.append(",\"ownedStatus\":\"").append(ownedStr).append("\"}");
        }
        prfs.clear();
    }
    result.append("]");
    status.setStatusCode(0, result);
    return;
}/*}}}*/

static void _bind(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& typeId = doc["typeId"];
    if (!typeId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& deviceId = doc["deviceId"];
    if (!deviceId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    int ret = deviceManager().SetDeviceOwnedStatus(deviceId.GetString(), HB_DEVICE_LOCAL_BINDED);

    if (0 == ret) {
        std::string result;
        result.append("\"result\":{\"typeId\":").append("\"").append(typeId.GetString()).append("\"");
        result.append(",\"deviceId\":").append("\"").append(deviceId.GetString()).append("\"");
        result.append(",\"ownedStatus\":").append("\"HB_binded\"}");
        status.setStatusCode(0, result);
        return;
    }
    status.setStatusCode(-1301);
    return ;
}/*}}}*/

static void _unbind(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& deviceId = doc["deviceId"];
    if (!deviceId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    int ret = deviceManager().SetDeviceOwnedStatus(deviceId.GetString(),  HB_DEVICE_UNBINDED);

    if (0 == ret) {
        std::string result;
        result.append("\"result\":{\"deviceId\":").append("\"").append(deviceId.GetString()).append("\"");
        result.append(",\"ownedStatus\":").append("\"unbinded\"}");
        status.setStatusCode(0, result);
        return;
    }
    status.setStatusCode(-1301);
    return ;
}/*}}}*/

static void _isonline(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& deviceId = doc["deviceId"];
    if (!deviceId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    bool online = false;
    int ret = deviceManager().GetDeviceOnlineStatus(deviceId.GetString(), online);
    if (ret ==0 && online)
        status.setStatusCode(0, "\"result\":{\"isonline\":\"true\"}");
    else
        status.setStatusCode(0, "\"result\":{\"isonline\":\"false\"}");

    return ;
}/*}}}*/

static void _profile(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    int ret;
    std::vector<DeviceTableInfo> prfs;
    DeviceTableInfo filter;
    if (doc.HasMember("typeId")) {
        Value& typeId = doc["typeId"];
        if (!typeId.IsString()) {
            status.setStatusCode(-1002);
            return;
        }
        filter.nDeviceType = typeId.GetString();
    } else {
        Value& deviceId = doc["deviceId"];
        if (!deviceId.IsString()) {
            status.setStatusCode(-1002);
            return;
        }
        ret = deviceManager().GetDevicesTypeById(deviceId.GetString(), filter.nDeviceType);
        if (ret != 0) {
            status.setStatusCode(-1501);
            return;
        }
    }
    mainDB().queryBy(prfs, filter);
    if (prfs.size() == 1) {
        std::string result("\"result\":{\"profile\":");
        result.append(prfs[0].nJsonData);
        result.append(",\"description\":\"null\"");
        result.append(",\"label\":\"").append(prfs[0].nDeviceName).append("\"");
        result.append(",\"iconid\":\"").append(prfs[0].nIconId).append("\"");
        result.append(",\"manufacture\":\"").append(prfs[0].nDeviceManu).append("\"");
        result.append(",\"version\":\"").append(prfs[0].nDeviceVer).append("\"");
        result.append("}");
        status.setStatusCode(0, result);
        return;
    }
    status.setStatusCode(-1502);
    return ;
}/*}}}*/

static void _status(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& deviceId = doc["deviceId"];
    if (!deviceId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    std::map<std::string, std::string> propertyList;
    int ret = deviceManager().GetDevicePropertyListWithValue(deviceId.GetString(), propertyList);
    if (ret != 0) {
        status.setStatusCode(-1601);
        return;
    }
    std::string result("\"result\":{");
    bool firstItem = true;
    for (auto it = propertyList.begin(); it != propertyList.end(); ++it) {
        if (firstItem) firstItem = false; else result.append(",");
        result.append("\"").append(it->first).append("\":");
        result.append("\"").append(it->second).append("\"");
    }
    result.append("}");
    status.setStatusCode(0, result);
    return ;
}/*}}}*/

static void _operate(const crow::request& req, crow::response& res)
{/*{{{*/
    DeviceStatusResult status(req, res);

    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError()) {
        status.setStatusCode(-1002);
        return;
    }
    Value& deviceId = doc["deviceId"];
    if (!deviceId.IsString()) {
        status.setStatusCode(-1002);
        return;
    }

    Value& propSet = doc["propSet"];
    if (!propSet.IsArray()) {
        status.setStatusCode(-1002);
        return;
    }

    std::map<std::string, std::string> propertyList;
    for (size_t i = 0; i < propSet.Size(); ++i) {
        Value &devAttr = propSet[i];
        if (!devAttr.IsObject()) {
            status.setStatusCode(-1002);
            return;
        }
        rapidjson::Value &name = devAttr["name"];
        if (!name.IsString()) {
            status.setStatusCode(-1002);
            return;
        }
        rapidjson::Value &value = devAttr["value"];
        if (!value.IsString()) {
            status.setStatusCode(-1002);
            return;
        }
        std::string val = value.GetString();
        if (val == "true")
            propertyList[name.GetString()] = "1";
        else if (val == "false")
            propertyList[name.GetString()] = "0";
        else
            propertyList[name.GetString()] = val;
    }

    int ret = deviceManager().SetDevicePropertyListWithValue(deviceId.GetString(), propertyList, true);

    if (0 == ret)
        status.setStatusCode(0, "\"result\": {\"ret\": \"SUCCESS\"}");
    else
        status.setStatusCode(-1701);

    return ;
}/*}}}*/

extern void initDevicesAPI(APP& app)
{
    CROW_ROUTE(app, "/api/familydevice/discovery").methods(REST_POST)(_discovery);
    CROW_ROUTE(app, "/api/familydevice/list").methods(REST_POST)(_list);
    CROW_ROUTE(app, "/api/familydevice/bind").methods(REST_POST)(_bind);
    CROW_ROUTE(app, "/api/familydevice/unbind").methods(REST_POST)(_unbind);
    CROW_ROUTE(app, "/api/familydevice/isonline").methods(REST_POST)(_isonline);
    CROW_ROUTE(app, "/api/familydevice/profile").methods(REST_POST)(_profile);
    CROW_ROUTE(app, "/api/familydevice/status").methods(REST_POST)(_status);
    CROW_ROUTE(app, "/api/familydevice/operate").methods(REST_POST)(_operate);
}

} /* namespace HB */
