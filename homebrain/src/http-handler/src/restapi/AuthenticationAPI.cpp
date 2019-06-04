/***************************************************************************
 *  AuthenticationAPI.cpp -
 *
 *  Created: 2018-10-25 20:38:17
 *
 *  Copyright QRS
 ****************************************************************************/

#include "AuthenticationAPI.h"
#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

namespace HB {


static void _login(const crow::request& req, crow::response& res)
{
    Document doc;
    doc.Parse(req.body.c_str());

    if (doc.HasParseError() ||
        !doc.HasMember("user") ||
        !doc.HasMember("pwd")) {
        HH_LOGE("rapidjson parse error: not found params!\n");
        res.write("{\"token\":\"adbcd123456\", \"status\":-1}");
        res.end();
        return;
    }
    Value& username = doc["user"];
    Value& password = doc["pwd"];
    if (strcmp(username.GetString(), "lidong") ||
        strcmp(password.GetString(), "1")) {
        res.write("{\"token\":\"adbcd123456\", \"status\":-2}");
        res.end();
        return;
    }
    res.write("{\"token\":\"adbcd123456\", \"status\":1}");
    res.end();
    return;
}

static void _logout(const crow::request& req, crow::response& res)
{
    res.write("{}");
    res.end();
}

extern void initAuthenticationAPI(APP& app)
{
    /* TODO test */

    CROW_ROUTE(app, "/api/authentication/userinfo")
        ([](){
         return "{\"user\": \"lidong\", \"logged_in\":true}";
         });

    CROW_ROUTE(app, "/api/authentication/login").methods(REST_POST)(_login);
    CROW_ROUTE(app, "/api/authentication/logout").methods(REST_POST)(_logout);
}

} /* namespace HB */
