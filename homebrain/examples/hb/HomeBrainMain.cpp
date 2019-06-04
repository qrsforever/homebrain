/***************************************************************************
 *  HomeBrainMain.cpp - HomeBrainMain
 *
 *  Created: 2018-06-13 15:25:27
 *
 *  Copyright
 ****************************************************************************/

#include "Log.h"
#include "LogFile.h"
#include "LogThread.h"
#include "LogPool.h"
#include "Message.h"
#include "MessageTypes.h"
#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "DeviceDataChannel.h"
#include "CloudDataChannel.h"

#include "MainPublicHandler.h" /* temp use, not rule module */
#include "MonitorTool.h"

#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

#ifndef SIM_SUITE
#include "HBDeviceManager.h"
#include "HBCloudManager.h"
#else
#include "TempSimulateSuite.h"
#endif

using namespace HB;
using namespace UTILS;

#ifndef SIM_SUITE
using namespace OIC::Service::HB;

class HBDeviceCallbackHandlerImpl *gDeviceCallback = 0;

class HBDeviceCallbackHandlerImpl : public HBDeviceCallBackHandler {
public:
    HBDeviceCallbackHandlerImpl(std::shared_ptr<DeviceDataChannel> devchnnl) : mDevChnnl(devchnnl) {}
    ~HBDeviceCallbackHandlerImpl(){}

    void onDeviceStatusChanged(const std::string deviceId, const std::string deviceType, HBDeviceStatus status) {
        if (status == HB_DEVICE_STATUS_ONLINE)
            mDevChnnl->statusChanged(deviceId, deviceType, 2);
        else if (status == HB_DEVICE_STATUS_OFFLINE)
            mDevChnnl->statusChanged(deviceId, deviceType, 3);
    }

    void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value) {
        mDevChnnl->propertyChanged(deviceId, propertyKey, value);
        cloudManager().propertyReport(deviceId, propertyKey, value);
    }
private:
    std::shared_ptr<DeviceDataChannel> mDevChnnl;
}; /* class HBDeviceCallbackHandlerImpl */
#endif

extern "C" int initMainThread();
extern "C" int mainThreadRun();

void _CatchSignal(int signo)
{
    switch (signo) {
        case SIGINT:
            monitor().finalize(); /* release socket resource */
            exit(0);
    }
}

/* init all modules thread */
class InitThread : public Thread {
public:
    InitThread(){}
    ~InitThread(){}
    void run();
};

void InitThread::run()
{
    printf("\n-----------Init Thread:[%u]---------\n", (unsigned int)pthread_self());

    /*-----------------------------------------------------------------
     *  Rule Engine module init
     *-----------------------------------------------------------------*/
    std::shared_ptr<DeviceDataChannel> deviceChnnl = std::make_shared<ElinkDeviceDataChannel>();
    ruleEngine().setServerRoot("clips");
    ruleEngine().setDeviceChannel(deviceChnnl);
    ruleEngine().init();

    /*-----------------------------------------------------------------
     *  Monitor Tool module init
     *-----------------------------------------------------------------*/
    printf("\nMonitor Init!\n");
    monitor().init(8192);
    monitor().start();
    signal(SIGINT, _CatchSignal);

    /*-----------------------------------------------------------------
     *  Device Manager module init
     *-----------------------------------------------------------------*/
    gDeviceCallback = new HBDeviceCallbackHandlerImpl(deviceChnnl);
    deviceManager().SetCallback(gDeviceCallback);
    deviceManager().Init();

#ifdef SIM_SUITE
    /*-----------------------------------------------------------------
     *  Simulate Test module init
     *-----------------------------------------------------------------*/
    mainHandler().sendMessageDelayed(mainHandler().obtainMessage(MT_SIMULATE, 0, 0), 1000);
#endif
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("\n-----------Main Thread:[%u]---------\n", (unsigned int)pthread_self());

    /*-----------------------------------------------------------------
     *  1. init log module
     *-----------------------------------------------------------------*/
    initLogThread();
    setLogLevel(LOG_LEVEL_WARNING);

    LogPool::getInstance().attachFilter(new LogFile());

    /*-----------------------------------------------------------------
     *  2. init main module
     *-----------------------------------------------------------------*/
    initMainThread();

    /*-----------------------------------------------------------------
     *  3. init others modules
     *-----------------------------------------------------------------*/
    InitThread init;
    init.start();

    /*-----------------------------------------------------------------
     *  4. dispach the message, never return;
     *-----------------------------------------------------------------*/
    return mainThreadRun();
}
