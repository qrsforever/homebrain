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
#include "MicroHttpHandler.h"
#include "DeviceDataChannel.h"
#include "CloudDataChannel.h"
#include "NotifyDataChannel.h"
#include "HBHelper.h"
#include "HBDatabase.h"
#include "HBGlobalTable.h"
#include "RuleEngineTable.h"
#include "SceneEngineTable.h"
#include "DeviceProfileTable.h"

#include "GatewayAgent.h"

#include "MainPublicHandler.h"
#ifdef ENABLE_MONITOR_TOOL
#include "MonitorTool.h"
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/prctl.h>

using namespace HB;
using namespace UTILS;
using namespace OIC::Service::HB;

/* ugly work around */
int s_illuminate = -1;

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
        else if (status == HB_DEVICE_STATUS_UNKOWN_ERROR)
            mDevChnnl->statusChanged(deviceId, deviceType, 4);
        /* cloudManager().statusReport(deviceId, deviceType, status); */
    }

    void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, std::string value) {
        mDevChnnl->propertyChanged(deviceId, propertyKey, value);

        /* std::string deviceType;
         * deviceManager().GetDevicesTypeById(deviceId, deviceType);
         * cloudManager().propertyReport(deviceId, deviceType, propertyKey, value); */

        /* if (propertyKey == "illuminate") {
         *     int around = atoi(value.c_str());
         *     if (s_illuminate == around)
         *         return;
         *     s_illuminate = around;
         * }
         * if ((propertyKey == "brightness") || (propertyKey == "ct") ||
         *     (propertyKey == "hue") || (propertyKey == "saturation"))
         *     return;
         * cloudManager().propertyReport(deviceId, propertyKey, value); */
    }
private:
    std::shared_ptr<DeviceDataChannel> mDevChnnl;
}; /* class HBDeviceCallbackHandlerImpl */

std::string gRootDir;

extern "C" int initMainThread();
extern "C" int mainThreadRun();

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
     *  log, attach
     *-----------------------------------------------------------------*/
    
    // disableLogPool();
    LogPool::getInstance().attachFilter(
        new LogFile(&mainHandler(), gRootDir.c_str(), 5120000));

    std::string clipsDir("clips");
    std::string deviceDir("devices/profiles");

    /*-----------------------------------------------------------------
     *  HomeBrain database init
     *-----------------------------------------------------------------*/
    mainDB().init(gRootDir, 15000);

    deviceProfileCheckAndUpdate(deviceDir.c_str());

    /* query log module level and set */
    std::vector<LogTableInfo> logs;
    mainDB().queryBy(logs, LogTableInfo());
    for (size_t i = 0; i < logs.size(); ++i)
        setModuleLogLevel(logs[i].nModule.c_str(), logs[i].nLevel);

    /* query devices and set */
    std::map<std::string, std::string> mapDevices;
    std::vector<DeviceTableInfo> devices;
    mainDB().queryBy(devices, DeviceTableInfo());
    for (size_t i = 0; i < devices.size(); ++i) {
        mapDevices.insert(std::pair<std::string, std::string>(
                devices[i].nDeviceType,
                devices[i].nScriptData));
    }

    /* query rules and set */
    std::map<std::string, std::string> mapRules;
    std::vector<RuleTableInfo> rules;
    RuleTableInfo filter;
    filter.nEnable = 1;
    mainDB().queryBy(rules, filter);
    for (size_t i = 0; i < rules.size(); ++i) {
        mapRules.insert(std::pair<std::string, std::string>(
                rules[i].nRuleId,
                rules[i].nScriptData));
    }
    rules.clear();

    /* query scenes and set */
    std::vector<SceneTableInfo> scenes;
    mainDB().queryBy(scenes, SceneTableInfo());
    for (size_t i = 0; i < scenes.size(); ++i) {
        mapRules.insert(std::pair<std::string, std::string>(
                scenes[i].nSceneId,
                scenes[i].nScriptData));
    }
    scenes.clear();

    /*-----------------------------------------------------------------
     *  Rule Engine module init
     *-----------------------------------------------------------------*/
    std::shared_ptr<DeviceDataChannel> deviceChnnl = std::make_shared<ElinkDeviceDataChannel>();
    ruleEngine().setServerRoot(clipsDir);
    ruleEngine().setDeviceChannel(deviceChnnl);
    ruleEngine().setNotifyChannel(std::make_shared<NotifyDataChannel>());
    ruleEngine().init(mapDevices, mapRules);
    mapDevices.clear();
    mapRules.clear();

    /* init cloud mananger here */
    /* cloudManager(); */

    /*-----------------------------------------------------------------
     *  Http Handler init
     *-----------------------------------------------------------------*/
    httpHandler().init(8899, 2);
    httpHandler().start();

#ifdef ENABLE_MONITOR_TOOL
    /*-----------------------------------------------------------------
     *  Monitor Tool module init
     *-----------------------------------------------------------------*/
    printf("\nMonitor Init!\n");
    monitor().init(8192);
    monitor().start();
#endif

    /*-----------------------------------------------------------------
     *  Device Manager module init
     *-----------------------------------------------------------------*/
    gDeviceCallback = new HBDeviceCallbackHandlerImpl(deviceChnnl);
    deviceManager().SetCallback(gDeviceCallback);
    deviceManager().Init();

    // TODO check network
    mainHandler().sendMessage(mainHandler().obtainMessage(MT_NETWORK,
            NETWORK_EVENT_CONNECT, NETWORK_CONNECT_SUCCESS));
}

int main(int argc, char *argv[])
{
    printf("\n-----------Main Thread:[%u]---------\n", (unsigned int)pthread_self());

    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
            case 'd':
                if (chdir(optarg) < 0)
                    perror("chdir");
                break;
            default:
                exit(1);
        }
    }

    char currdir[128];
    gRootDir.assign(getcwd(currdir, sizeof(currdir)));

    /*-----------------------------------------------------------------
     *  1. init log module
     *-----------------------------------------------------------------*/
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);

    /*-----------------------------------------------------------------
     *  2. init main module
     *-----------------------------------------------------------------*/
    initMainThread();

    /*-----------------------------------------------------------------
     *  3. init others modules
     *-----------------------------------------------------------------*/
    InitThread init; init.start();

    /*-----------------------------------------------------------------
     *  4. dispach the message, never return;
     *-----------------------------------------------------------------*/
    return mainThreadRun();
}
