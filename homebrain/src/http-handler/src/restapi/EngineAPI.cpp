/***************************************************************************
 *  EngineAPI.cpp -
 *
 *  Created: 2019-01-17 19:25:58
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"
#include "HBDatabase.h"
#include "Common.h"
#include "HBGlobalTable.h"

#include <sstream>
#include <fstream>

using namespace UTILS;
using namespace crow;

#define WWW_DIR "./www"

namespace HB {

static std::map<int, const char*> gErrorCodes {/*{{{*/
    {-1000, "engine api common error"},
    {-1001, "check token error"},
    {-1002, "document parse error"},
};/*}}}*/

class EngineStatusResult {/*{{{*/
public:
    EngineStatusResult(const crow::request& req, crow::response& res)
        : mReq(req), mRes(res), mCode(0), mResext("") {}

    ~EngineStatusResult();

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

EngineStatusResult::~EngineStatusResult()
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

static std::string& getLocalesJson()
{
    static std::string s_jsonData;
    if (s_jsonData.empty()) {
        std::string filename(WWW_DIR);
        filename.append("/assets/locales/en.json");
        std::ifstream is(filename);
        s_jsonData.assign((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    }
    return s_jsonData;
}

static void _sysinfo(const crow::request& req, crow::response& res)
{
    EngineStatusResult status(req, res);

    std::vector<SmartHomeInfo> infos;
    mainDB().queryBy(infos, SmartHomeInfo());
    std::string items("\"data\":{");
    for (size_t i, l = infos.size(); i < l; ++i) {
        if (i != 0) items.append(",");
        items.append("\"").append(infos[i].nName).append("\":");
        items.append("\"").append(infos[i].nValue).append("\"");
    }
    items.append("}");
    status.setStatusCode(0, items);
    return;
}

static void _root(const crow::request& /*req*/, crow::response& res)
{
    mustache::context ctx(json::load(getLocalesJson()));
    res.write(mustache::load("index.html").render(ctx));
    res.end();
}

extern void initEngineAPI(APP& app)
{
    crow::mustache::set_base(WWW_DIR);

    CROW_ROUTE(app, "/api/sysinfo")(_sysinfo);

    CROW_ROUTE(app, "/")(_root);

    CROW_ROUTE(app, "/js/<path>")
        ([](std::string file) {
            std::string jsfile("js/");
            jsfile.append(file);
            mustache::context ctx(json::load(getLocalesJson()));
            return crow::mustache::load(jsfile).render(ctx);
         });

    CROW_ROUTE(app, "/<path>")
        ([](std::string file) {
            return crow::mustache::load(file).render();
         });
}

} /* namespace HB */
