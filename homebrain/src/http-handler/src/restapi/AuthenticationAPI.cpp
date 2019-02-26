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

namespace HB {

static void _login(const crow::request& req, crow::response& res)
{
    res.write("{\"token\":\"adbcd123456\"}");
    res.end();
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
         return "{\"user\": \"lidong8@le.com\", \"logged_in\":true}";
         });

    CROW_ROUTE(app, "/api/authentication/login").methods(REST_POST)(_login);
    CROW_ROUTE(app, "/api/authentication/logout").methods(REST_POST)(_logout);
}

} /* namespace HB */
