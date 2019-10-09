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
#include "CurlEasy.h"
#include "Common.h"

#include "GatewayConnectTable.h"
#include "HBGlobalTable.h"

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

void MainPublicHandler::onLogEvent(Message *msg)
{
    switch (msg->arg1) {
        case LOG_EVENT_FILE_CREATE:
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
                std::vector<SmartHomeInfo> infos;
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
        default:
            ;
    }
}

void MainPublicHandler::doNetworkSuccess()
{
    /* query bridge and start */
    std::vector<GatewayTableInfo> bridges;
    GatewayTableInfo filter;
    filter.nEnable = 1;
    mainDB().queryBy(bridges, filter);
    for (size_t i = 0; i < bridges.size(); ++i) {
        startBridge(bridges[i].nType.c_str(), bridges[i].nGid.c_str(),
            bridges[i].nKey.c_str(), bridges[i].nIp.c_str());
    }
}

void MainPublicHandler::onNetworkEvent(Message *msg)
{
    if (msg->arg1 != NETWORK_EVENT_CONNECT)
        return;
    switch (msg->arg2) {
        case NETWORK_CONNECT_SUCCESS:
            doNetworkSuccess();
            break;
        default:
            break;
    }
}

void MainPublicHandler::onDeviceEvent(Message *msg)
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
void MainPublicHandler::onMonitorEvent(Message *msg)
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

void MainPublicHandler::handleMessage(Message *msg)
{
    LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);
    switch (msg->what) {
        case MT_LOG:
            onLogEvent(msg);
            break;
        case MT_NETWORK:
            onNetworkEvent(msg);
            break;
        case MT_DB:
            if (msg->arg1 == DB_CLOSE)
                mainDB().close();
            break;
        case MT_RULE:
            break;
        case MT_DEVICE:
            onDeviceEvent(msg);
            break;
#ifdef ENABLE_MONITOR_TOOL
        case MT_MONITOR:
            onMonitorEvent(msg);
            break;
#endif
        default:
            break;
    }
}

MainPublicHandler& mainHandler()
{
    if (0 == gMainHander) {
        LOGI("check me\n");
        gMainHander = new MainPublicHandler(gMainThread->getMessageQueue());
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
