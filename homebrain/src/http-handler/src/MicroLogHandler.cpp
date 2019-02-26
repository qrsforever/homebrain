/***************************************************************************
 *  MicroLogHandler.cpp -
 *
 *  Created: 2018-10-24 13:52:30
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MicroLogHandler.h"

int gHttpHandlerModuleLevel = LOG_LEVEL_DEBUG;

namespace HB {

static LogModule HttpHandlerModule("http-handler", gHttpHandlerModuleLevel);

MicroLogHandler::MicroLogHandler()
{
}

MicroLogHandler::~MicroLogHandler()
{
}

void MicroLogHandler::log(std::string message, crow::LogLevel level)
{
    switch (level) {
        case crow::LogLevel::DEBUG:
            HH_LOGD("%s", message.c_str());
            break;
        case crow::LogLevel::INFO:
            HH_LOGI("%s", message.c_str());
            break;
        case crow::LogLevel::WARNING:
            HH_LOGW("%s", message.c_str());
            break;
        case crow::LogLevel::ERROR:
            HH_LOGE("%s", message.c_str());
            break;
        default:
            ;
    }
}

} /* namespace HB */
