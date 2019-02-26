/***************************************************************************
 *  MainPublicHandler.h - Main public handler
 *
 *  Created: 2018-06-13 15:20:34
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __MainPublicHandler_H__
#define __MainPublicHandler_H__

#include "MessageHandler.h"

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class MainPublicHandler : public MessageHandler {
public:
    MainPublicHandler();
    MainPublicHandler(MessageQueue *queue);
    ~MainPublicHandler();

protected:
    void handleMessage(Message *msg);

private:
    void doDeviceEvent(Message *msg);
#ifdef ENABLE_MONITOR_TOOL
    void doMonitorEvent(Message *msg);
#endif
#ifdef SIM_SUITE
    void doSimulateEvent(Message *msg);
#endif

}; /* class MainPublicHandler */

MainPublicHandler& mainHandler();

/* int initMainThread(); */
/* int mainThreadRun(); */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __MainPublicHandler_H__ */
