/***************************************************************************
 *  MainPublicHandler.cpp - Main public handler impl
 *
 *  Created: 2018-06-13 15:21:05
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MainPublicHandler.h"
#include "Log.h"
#include "StringData.h"
#include "MessageTypes.h"
#include "MessageLooper.h"
#include "RuleEventHandler.h"
#include "RuleEngineService.h"
#include "HBDatabase.h"
#include "HBDeviceManager.h"
#include "HBHelper.h"

#ifdef ENABLE_MONITOR_TOOL
#include "MonitorTool.h"
#endif

#ifdef SIM_SUITE
#include "TempSimulateSuite.h" /* TODO only for test */
#endif

namespace HB {

static MessageLooper *gMainThread = 0;
static MainPublicHandler *gMainHander = 0;

MainPublicHandler::MainPublicHandler()
{

}

MainPublicHandler::MainPublicHandler(MessageQueue *queue)
    : MessageHandler(queue)
{

}

MainPublicHandler::~MainPublicHandler()
{

}

void MainPublicHandler::doDeviceEvent(Message *msg)
{
    switch (msg->arg1) {
        case DEVICE_STATUS_UNKOWN:
            deviceManager().Deinit();
            /* TODO will block */
            deviceManager().Init();
            break;
        default:
            break;
    }
}

#ifdef ENABLE_MONITOR_TOOL
void MainPublicHandler::doMonitorEvent(Message *msg)
{
    switch (msg->arg1) {
        case MONITOR_CLOSE_CLIENT:
            monitor().delClient(msg->arg2);
            break;
        default:
            ;
    }
}
#endif

#ifdef SIM_SUITE
void MainPublicHandler::doSimulateEvent(Message *msg)
{
    /* TODO only for test */
    tempSimulateTest(msg);
}
#endif

void MainPublicHandler::handleMessage(Message *msg)
{
    LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);
    switch (msg->what) {
        case MT_DB:
            if (msg->arg1 == DB_CLOSE)
                mainDB().close();
            break;
        case MT_RULE:
            break;
        case MT_DEVICE:
            doDeviceEvent(msg);
            break;
#ifdef ENABLE_MONITOR_TOOL
        case MT_MONITOR:
            doMonitorEvent(msg);
            break;
#endif
#ifdef SIM_SUITE
        case MT_SIMULATE:
            doSimulateEvent(msg);
            break;
#endif
        default:
            break;
    }
}

MainPublicHandler& mainHandler()
{
    if (0 == gMainHander) {
        LOGW("check me\n");
        gMainHander = new MainPublicHandler(gMainThread->getMessageQueue());
    }
    return *gMainHander;
}

extern "C" {

int initMainThread()
{
    if (0 == gMainThread) {
        /* TODO call this API in main thread */
        gMainThread = new MessageLooper(pthread_self());
        gMainHander = new MainPublicHandler();
    }
    return 0;
}

int mainThreadRun()
{
    if (gMainThread)
        gMainThread->run();
    return 0;
}

} /* extern C */

} /* namespace HB */
