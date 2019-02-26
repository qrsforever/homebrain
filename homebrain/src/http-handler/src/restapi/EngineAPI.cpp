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
#include "StringArray.h"
#include "StringData.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "CloudDataChannel.h"
#include "RuleEngineTable.h"

#include <sstream>
#include <fstream>

using namespace UTILS;
using namespace crow;

#define WWW_DIR "./www"

namespace HB {

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

static void _root(const crow::request& /*req*/, crow::response& res)
{
    mustache::context ctx(json::load(getLocalesJson()));
    res.write(mustache::load("index.html").render(ctx));
    res.end();
}

extern void initEngineAPI(APP& app)
{
    crow::mustache::set_base(WWW_DIR);

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
