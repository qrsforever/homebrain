/***************************************************************************
 *  MainHandler.cpp -
 *
 *  Created: 2019-06-18 13:45:19
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MainHandler.h"
#include "LFDatabase.h"
#include "Log.h"
#include "MessageTypes.h"
#include "MessageLooper.h"
#include "CurlEasy.h"
#include "DBTables.h"
#include "StringData.h"
#include "StringArray.h"
#include "Common.h"
#include "LFHelper.h"

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

void MainHandler::onLogEvent(Message *msg)
{
    switch (msg->arg1) {
        case LOG_EVENT_FILE_CREATE:
            if (msg->obj) {
#ifdef USE_SHARED_PTR
                std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
#else
                StringData *data = dynamic_cast<StringData*>(msg->obj);
#endif
                if (data)
                    gLogFile.assign(data->getData());
            }
            break;
        case LOG_EVENT_FILE_READY:
            if (msg->obj) {
                std::string filename;
#ifdef USE_SHARED_PTR
                std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
#else
                StringData *data = dynamic_cast<StringData*>(msg->obj);
#endif
                if (data)
                    filename.assign(data->getData());
                std::vector<LFGlobalInfo> infos;
                mainDB().queryBy(infos, LOG_FTP_FIELD);
                if (infos.size() == 1) {
                    std::vector<std::string> params = UTILS::stringSplit(infos[0].nValue, "^");
                    if (params.size() == 3) {
                        LOGD("ftp upload\n");
                        //TODO need do this in another thread.
                        easyPut(params[0].c_str(), filename.c_str(), params[1].c_str(), params[2].c_str());
                    }
                }
                unlink(filename.c_str());
            }
            break;
        case LOG_EVENT_FILE_UPLOAD:
            if (msg->obj)  {
#ifdef USE_SHARED_PTR
                std::shared_ptr<StringArray> data(std::dynamic_pointer_cast<StringArray>(msg->obj));
#else
                StringArray *data = dynamic_cast<StringArray*>(msg->obj);
#endif
                if (LFLogUpload(gLogFile, data->get(0), data->get(1), data->get(2)) < 0)
                    LOGW("upload log error!\n");
            }
            break;
        default:
            ;
    }
}

void MainHandler::onSystemEvent(Message *msg)
{
    switch (msg->arg1) {
        case SYSTME_EVENT_RECOVERY:
            do {
                // dump current info
                LFDumpInfo();

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
        case MT_LOG:
            return onLogEvent(msg);
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
