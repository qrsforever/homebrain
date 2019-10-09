/***************************************************************************
 *  MicroHttpHandler.cpp -
 *
 *  Created: 2018-10-24 13:38:46
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MicroHttpHandler.h"
#include "MicroLogHandler.h"

namespace HB {

extern void initEngineAPI(APP&);
extern void initAuthenticationAPI(APP&);
extern void initGatewaysAPI(APP&);
extern void initDevicesAPI(APP&);
extern void initRulesAPI(APP&);
extern void initScenesAPI(APP&);

MicroHttpHandler::MicroHttpHandler()
    : Thread()
    , mPort(8899)
    , mMulti(1)
{
}

MicroHttpHandler::~MicroHttpHandler()
{
}

void MicroHttpHandler::init(int port, int multi)
{
    mPort = port;
    mMulti = multi;
}

void MicroHttpHandler::run()
{
    crow::App<MicroMiddleware> app;

    initAuthenticationAPI(app);
    initGatewaysAPI(app);
    initDevicesAPI(app);
    initRulesAPI(app);
    initScenesAPI(app);
    initEngineAPI(app);

    MicroLogHandler *logger = new MicroLogHandler();
    crow::logger::setLogLevel(crow::LogLevel::DEBUG);
    crow::logger::setHandler(logger);

    app.port(mPort).concurrency(mMulti).run();
}

int checkToken(const std::string &token)
{
    if (token.empty())
        return 2;
    return 0;
}

MicroHttpHandler& httpHandler()
{
    static MicroHttpHandler *gHttpHandler = 0;
    if (gHttpHandler == 0)
        gHttpHandler = new MicroHttpHandler();
    return *gHttpHandler;
}

} /* namespace HB */
