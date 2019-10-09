/***************************************************************************
 *  MainHandler.cpp -
 *
 *  Created: 2019-09-27
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MainHandler.h"
#include "KKDatabase.h"
#include "Log.h"
#include "MessageTypes.h"
#include "MessageLooper.h"
#include "CurlEasy.h"
#include "DBTables.h"
#include "StringData.h"
#include "StringArray.h"
#include "Common.h"
#include "KKHelper.h"

#include "GatewayAgentHandler.h"
#include "GatewayAgent.h"

#include <unistd.h>

namespace HB {

static MessageLooper *gMainThread = 0;
static MainHandler *gMainHander = 0;
static std::string gLogFile = "";

MainHandler::MainHandler()
{

}

MainHandler::MainHandler(MessageQueue *queue)
    : MessageHandler(queue)
{

}

MainHandler::~MainHandler()
{

}

void MainHandler::onSystemEvent(Message *msg)
{
    switch (msg->arg1) {
        case SYSTME_EVENT_RECOVERY:
            do {
                // dump current info
                KKDumpInfo();

                // TODO wait other module do recovery event
                sendMessageDelayed(obtainMessage(
                        MT_SYSTEM, SYSTME_EVENT_REBOOT, 0), 3000);
            } while(0);
            break;
        case SYSTME_EVENT_REBOOT:
            CRASH();
            break;
        default:
            ;
    }
}

void MainHandler::onDatabaseEvent(Message *msg)
{
    if (msg->arg1 == DB_CLOSE)
        mainDB().close();
}

void MainHandler::handleMessage(Message *msg)
{
    LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);
    if (msg->what ==  MT_TIMER) {
        sendEmptyMessageDelayed(MT_TIMER, 1000);
        return;
    }
    switch (msg->what) {
        case MT_SYSTEM:
            return onSystemEvent(msg);
        case MT_DB:
            return onDatabaseEvent(msg);
        default:
            break;
    }
}

MainHandler& mainHandler()
{
    if (0 == gMainHander) {
        LOGI("check me\n");
        gMainHander = new MainHandler(gMainThread->getMessageQueue());
    }
    return *gMainHander;
}

extern "C" {

int initMainThread()
{
    if (0 == gMainThread) {
        /* TODO call this API in main thread */
        gMainThread = &Looper::getMainLooper();
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
