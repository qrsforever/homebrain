/***************************************************************************
 *  MicroLogHandler.h -
 *
 *  Created: 2018-10-24 13:47:59
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __MicroLogHandler_H__
#define __MicroLogHandler_H__

#include "Log.h"
#include "crow.h"

extern int gHttpHandlerModuleLevel;

#define HH_LOGE(args...) _LOGE(gHttpHandlerModuleLevel, args)
#define HH_LOGW(args...) _LOGW(gHttpHandlerModuleLevel, args)
#define HH_LOGD(args...) _LOGD(gHttpHandlerModuleLevel, args)
#define HH_LOGI(args...) _LOGI(gHttpHandlerModuleLevel, args)

namespace HB {

class MicroLogHandler : public crow::ILogHandler {
public:
    MicroLogHandler();
    ~MicroLogHandler();

    void log(std::string message, crow::LogLevel level);
private:
}; /* class MicroLogHandler */

} /* namespace HB */

#endif /* __MicroLogHandler_H__ */

