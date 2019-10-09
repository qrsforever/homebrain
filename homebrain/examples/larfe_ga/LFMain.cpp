/***************************************************************************
 *  LFMain.cpp -
 *
 *  Created: 2019-06-17 22:24:13
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"
#include "LogFile.h"
#include "LogThread.h"
#include "LogPool.h"
#include "CurlEasy.h"
#include "StringData.h"
#include "StringArray.h"
#include "LarfeClient.h"
#include "Message.h"
#include "LFTypes.h"
#include "MainHandler.h"
#include "GatewayAgent.h"
#include "GatewayAgentHandler.h"
#include "LFDatabase.h"
#include "DBTables.h"
#include "LFHelper.h"
#if defined(LFGA_UNITTEST)
#include "LFUnitTest.h"
#endif

#ifdef ENABLE_MONITOR_TOOL
#include "LFMonitorTool.h"
#endif

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include <map>
#include <fstream>
#include <sstream>

using namespace HB;
using namespace UTILS;
using namespace rapidjson;

static LFDeviceInfo gLFHostInfo;
static MessageHandler::Callback* gStateBoot = 0;
static MessageHandler::Callback* gStateNormal = 0;

static std::map<std::string, std::string> gEidDids;
static std::map<std::string, std::string> gDidEids;
static std::string gWorkspaceDir = "/tmp";
static std::string gHostInfoFile = "hostinfo.txt";
static std::string gSystemCmd = "system.cmd";

std::string getDidByEid(const std::string &eid)
{/*{{{*/
    std::string did("");
    std::map<std::string, std::string>::iterator it = gEidDids.find(eid);
    if (it == gEidDids.end()) {
        std::vector<LFDeviceInfo> devs;
        LFDeviceInfo filter;
        filter.nEid = eid;
        mainDB().queryBy(devs, filter);
        if (devs.size() == 1) {
            gEidDids.insert(std::pair<std::string, std::string>(eid, devs[0].nDid));
            did = devs[0].nDid;
        }
    } else
        did = it->second;
    return did;
}/*}}}*/

std::string getEidByDid(const std::string &did)
{/*{{{*/
    std::string eid("");
    std::map<std::string, std::string>::iterator it = gDidEids.find(did);
    if (it == gDidEids.end()) {
        std::vector<LFDeviceInfo> devs;
        LFDeviceInfo filter;
        filter.nDid = did;
        mainDB().queryBy(devs, filter);
        if (devs.size() == 1) {
            gDidEids.insert(std::pair<std::string, std::string>(did, devs[0].nEid));
            eid = devs[0].nEid;
        }
    } else
        eid = it->second;
    return eid;
}/*}}}*/

class GatewayAgentThread: public ::UTILS::Thread {/*{{{*/
public:
    GatewayAgentThread(GatewayAgent &ga) : mQuit(false), mGA(ga) {}
    ~GatewayAgentThread (){}
    void run() {
        int errCount = 0;
        mStatus = 1;
        while (!mQuit) {
            if (mGA.yield(300) < 0) {
                errCount++;
                LOGW("yield error\n");
                usleep(500000);
                if (errCount == 30) {
                    mainHandler().sendMessage(
                        mainHandler().obtainMessage(LFMT_HB_GAGENT, HB_GAGENT_CONNECT, 0));
                    break;
                }
            }
        }
        mStatus = 0;
    }
    void stop() {
        mQuit = true;
        while (mStatus)
            usleep(100000);
    }
private:
    int mStatus;
    bool mQuit;
    GatewayAgent &mGA;
};/*}}}*/

void startGatewayAgentThread()
{/*{{{*/
    static GatewayAgentThread* sGAThread = 0;
    if (sGAThread) {
        sGAThread->stop();
        delete (sGAThread);
    }
    sGAThread = new GatewayAgentThread(GAgent());
    sGAThread->start();
}/*}}}*/

class LFDeviceCallback : public deviceCallBackHandler {/*{{{*/
    void onDeviceStatusChanged(const std::string deviceId, const std::string deviceName,
        const std::string productKey, DeviceType dtype, const std::string room, const std::string /*status*/) {
        LOGD("(%s, %s, %s, %d, %s)\n", deviceId.c_str(), deviceName.c_str(), productKey.c_str(), dtype, room.c_str());
        std::vector<LFDeviceInfo> devs;
        mainDB().queryBy(devs, deviceId);
        int arg1 = -1;
        if (devs.size() == 0) {
            LFDeviceInfo dev;
            dev.nDid = deviceId;
            dev.nName = deviceName;
            dev.nKey = productKey;
            dev.nRoom = room;
            dev.nTopo = 0;
            dev.nType = (int)dtype;
            mainDB().updateOrInsert(dev);
            arg1 = HB_GAGENT_REGISTER_SUBDEV;
        } else {
            if (1 != devs[0].nTopo)
                arg1 = HB_GAGENT_SUBDEV_TOPOADD;
        }
        if (-1 != arg1) {
#ifdef USE_SHARED_PTR
            std::shared_ptr<StringData> data = std::make_shared<StringData>(deviceId.c_str());
#else
            StringData *data = new StringData(deviceId.c_str());
#endif
            mainHandler().sendMessage(mainHandler().obtainMessage(LFMT_HB_GAGENT, arg1, 0, data));
#ifndef USE_SHARED_PTR
            data->safeUnref();
#endif
        }
    }

    void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, const std::string value) {
        LOGD("(%s, %s, %s)\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
        GAgent().eventReport(getEidByDid(deviceId), propertyKey, "", "status", value);
    }
};/*}}}*/

class LFRemoteEventCallback : public RemoteEventCallback {/*{{{*/
public:
    LFRemoteEventCallback(): mFirstFlg(true) { }
    ~LFRemoteEventCallback() {}

    int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) {
        if (elinkId.empty())
            return -1;
        LOGD("(%s %s %s)\n", deviceId.c_str(), deviceSecret.c_str(), elinkId.c_str());
        std::vector<LFDeviceInfo> devs;
        mainDB().queryBy(devs, deviceId);
        if (devs.size() == 1) {
            devs[0].nEid = elinkId;
            devs[0].nSecret = deviceSecret;
            mainDB().updateOrInsert(devs[0]);
#ifdef USE_SHARED_PTR
            std::shared_ptr<StringData> data = std::make_shared<StringData>(deviceId.c_str());
#else
            StringData *data = new StringData(deviceId.c_str());
#endif
            mainHandler().sendMessage(
                mainHandler().obtainMessage(LFMT_HB_GAGENT, HB_GAGENT_SUBDEV_TOPOADD, 0, data));
#ifndef USE_SHARED_PTR
            data->safeUnref();
#endif
        }
        return 0;
    }

    int doTopoAddReply(std::string elinkId, int code, std::string message) {
        LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
        std::vector<LFDeviceInfo> devs;
        LFDeviceInfo filter;
        filter.nEid = elinkId;
        mainDB().queryBy(devs, filter);
        if (devs.size() == 1) {
            devs[0].nTopo = 1;
            mainDB().updateOrInsert(devs[0]);
        }
        return 0;
    }

    int doTopoDelReply(std::string elinkId, int code, std::string message) {
        LOGD("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
        return -1;
    }

    int doPropertySet(std::string rid, std::string elinkId, std::vector<std::pair<std::string, std::string>>& params) {
        std::string did = getDidByEid(elinkId);
        if (did.empty()) {
            LOGW("not found elinkid[%s]\n", elinkId.c_str());
            return -1;
        }
        for (size_t i = 0; i < params.size(); ++i) {
            LOGD("%s: %s %s\n", did.c_str(), params[i].first.c_str(), params[i].second.c_str());
            if (0 != deviceManager().setDeviceProperty(did, params[i].first, params[i].second)) {
                GAgent().propertySetResult(rid, elinkId, 222, "call api error");
                return -1;
            }
        }
        GAgent().propertySetResult(rid, elinkId, 200, "success");
        return 0;
    }

    int doPropertyGet(std::string rid, std::string elinkId, std::vector<std::string> &params) {
        std::string did = getDidByEid(elinkId);
        if (did.empty()) {
            LOGW("not found elinkid[%s]\n", elinkId.c_str());
            return -1;
        }
        std::vector<std::pair<std::string, std::string>> props;
        for (size_t i = 0; i < params.size(); ++i) {
            LOGD("%s: %s\n", elinkId.c_str(), params[i].c_str());
            std::string value;
            VarType type;
            if (0 != deviceManager().getDeviceProperty(did, params[i], value, type)) {
                GAgent().propertyGetResult(rid, elinkId, 222, props);
                return -1;
            }
            props.push_back(std::make_pair(params[i], value));
        }
        GAgent().propertyGetResult(rid, elinkId, 200, props);
        return 0;
    }

    int doUpgradeReply(std::string newVersion, int code, std::string message) {
        LOGD("(%s, %d, %s)\n", newVersion.c_str(), code, message.c_str());
#ifdef USE_SHARED_PTR
        std::shared_ptr<StringData> data = std::make_shared<StringData>(newVersion.c_str());
#else
        StringData *data = new StringData(newVersion.c_str());
#endif
        mainHandler().sendMessage(
            mainHandler().obtainMessage(LFMT_HB_UPGRADE, HB_UPGRADE_BURNING, 0, data));
#ifndef USE_SHARED_PTR
        data->safeUnref();
#endif
        return 0;
    }

    int doUpgradeCheck(std::string text) {
        LOGD("(%s)\n", text.c_str());
        mainHandler().removeMessages(LFMT_HB_GAGENT, HB_GAGENT_UPGRADE_CHECK, 0);
        mainHandler().sendMessage(mainHandler().obtainMessage(
                LFMT_HB_GAGENT, HB_GAGENT_UPGRADE_CHECK, 1 /* force report */));
        return 0;
    }

    int doLogReport(std::string elinkId, std::string key, std::string logId) {
        LFDumpInfo(); /* don't remove this line, just let log file flush */
#ifdef USE_SHARED_PTR
        std::shared_ptr<StringArray> data = std::make_shared<StringArray>();
#else
        StringArray *data = new StringArray();
#endif
        if (!data)
            return -1;
        data->put(0, elinkId.c_str());
        data->put(1, key.c_str());
        data->put(2, logId.c_str());

        mainHandler().sendMessageDelayed(mainHandler().obtainMessage(
                MT_LOG, LOG_EVENT_FILE_UPLOAD, 0, data), 500);
#ifndef USE_SHARED_PTR
        data->safeUnref();
#endif
        return 0;
    }

    int doSystemCmd(std::string cmd, std::string /*params*/) {
        LOGA("(%s)\n", cmd.c_str());
        if (cmd == "recovery") {
            std::ofstream of(gSystemCmd.c_str());
            if (of.is_open()) {
                of << cmd;
                of.close();
            }
            mainHandler().sendMessage(mainHandler().obtainMessage(
                    MT_SYSTEM, SYSTME_EVENT_RECOVERY, 0));
        }
        return 0;
    }

private:
    bool mFirstFlg;
};/*}}}*/

class BootStateCallback: public ::UTILS::MessageHandler::Callback {/*{{{*/
public:
    bool handleMessage(Message *msg) {
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);
        switch (msg->what) {
            case MT_NETWORK:
                if (msg->arg2 == NETWORK_CONNECT_SUCCESS) {
                    if (!gLFHostInfo.nEid.empty()) {
                        mainHandler().sendMessage(
                            mainHandler().obtainMessage(
                                LFMT_HB_REGISTER, HB_REGISTER_EVENT_SUCCESS, 0));
                        return false;
                    }
                    mainHandler().sendMessage(
                        mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_REQUEST, 0));
                    return false;
                }
                break;
            case LFMT_HB_REGISTER:
                switch (msg->arg1) {
                    case HB_REGISTER_EVENT_REQUEST:
                        do {
                            LOGI("LFMT_HB_REGISTER.HB_REGISTER_EVENT_REQUEST\n");
                            std::string payload;
                            payload.append("{\"productKey\": \"").append(gLFHostInfo.nKey).append("\",");
                            payload.append("\"deviceName\": \"").append(gLFHostInfo.nName).append("\",");
                            payload.append("\"deviceId\": \"").append(gLFHostInfo.nDid).append("\",");
                            payload.append("\"manufacture\": \"").append("letv\"}");
                            payload = UTILS::easyPost(ELINK_CLOUD_HOST, payload.c_str(), payload.size());
                            if (payload.empty()) {
                                mainHandler().sendMessage(
                                    mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_FAIL, 0));
                                return false;
                            }
                            LOGD("payload: [%s]\n", payload.c_str());
                            Document doc;
                            doc.Parse<0>(payload.c_str());
                            if (doc.HasParseError() || !doc.HasMember("data")) {
                                LOGW("payload doc parse error!");
                                mainHandler().sendMessage(
                                    mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_FAIL, 1));
                                return false;
                            }
                            Value &data = doc["data"];
                            if (!data.HasMember("deviceId") || !data.HasMember("elinkId") ||
                                !data.HasMember("deviceSecret") || !data.HasMember("productKey")) {
                                LOGW("payload doc.data parse error!");
                                mainHandler().sendMessage(
                                    mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_FAIL, 2));
                                return false;
                            }
                            Value &elinkId = data["elinkId"];
                            Value &deviceSecret = data["deviceSecret"];
                            gLFHostInfo.nEid = elinkId.GetString();
                            gLFHostInfo.nSecret = deviceSecret.GetString();
                            mainDB().updateOrInsert(gLFHostInfo);
                            mainHandler().sendMessage(
                                mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_SUCCESS, 0));
                        } while (0);
                        break;
                    case HB_REGISTER_EVENT_SUCCESS:
                        LOGI("LFMT_HB_REGISTER.HB_REGISTER_EVENT_SUCCESS\n");
                        mainHandler().mCallback = gStateNormal;
                        GAgent().init(ELINK_EMQ_HOST, gLFHostInfo.nEid,
                            gLFHostInfo.nEid, gLFHostInfo.nSecret, gLFHostInfo.nKey);
                        mainHandler().sendMessage(
                            mainHandler().obtainMessage(LFMT_HB_GAGENT, HB_GAGENT_CONNECT, 0));
                        break;
                    case HB_REGISTER_EVENT_FAIL:
                        LOGW("LFMT_HB_REGISTER.HB_REGISTER_EVENT_FAIL\n");
                        mainHandler().sendMessageDelayed(
                            mainHandler().obtainMessage(LFMT_HB_REGISTER, HB_REGISTER_EVENT_REQUEST, 0), 10000);
                        break;
                }
                break;
            default:
                ;
        }
        return false; // don't return true;
    }
};/*}}}*/

class NormalStateCallback: public ::UTILS::MessageHandler::Callback {/*{{{*/
public:
    bool handleMessage(Message *msg) {
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);
        switch (msg->what) {
            case LFMT_HB_GAGENT:
                switch(msg->arg1) {
                    case HB_GAGENT_CONNECT:
                        if (GAgent().connect() < 0) {
                            LOGW("GAgent connect server error!\n");
                            mainHandler().sendMessageDelayed(
                                mainHandler().obtainMessage(LFMT_HB_GAGENT, HB_GAGENT_CONNECT, 0), 5000);
                        } else {
                            startGatewayAgentThread();
                            if (!mainHandler().hasMessages(LFMT_HB_GAGENT, HB_GAGENT_UPGRADE_CHECK, 0)) {
                                deviceManager().init();
                                mainHandler().sendMessage(mainHandler().obtainMessage(
                                        LFMT_HB_GAGENT, HB_GAGENT_UPGRADE_CHECK, 1 /* force report */));
                            }
                        }
                        break;
                    case HB_GAGENT_UPGRADE_CHECK:
                        do {
                            LOGI("HB_GAGENT_UPGRADE_CHECK\n");
                            std::string curVersion = LFBuildVersion();
                            std::string newVersion = "";
                            if (1 == LFOTAGetVersion(curVersion, newVersion) || msg->arg2 == 1) {
                                if (newVersion.empty())
                                    newVersion = curVersion;
                                LOGA("LF version [%s] vs [%s], git [%s] [%s]\n",
                                    curVersion.c_str(), newVersion.c_str(), LFBuildVersion(), LFBuildDatetime());
                                GAgent().upgradeReport(curVersion, newVersion);
                                std::vector<LFGlobalInfo> infos;
                                mainDB().queryBy(infos, CUR_VERSION_FIELD);
                                if (infos.size() == 1 && infos[0].nValue != curVersion) {
                                    infos[0].nValue = curVersion;
                                    mainDB().updateOrInsert(infos[0]);
                                    /* after upgrade new version, then update hostinfo file, maybe diff */
                                    mainHandler().sendMessage(
                                        mainHandler().obtainMessage(LFMT_HB_UPGRADE, HB_UPGRADE_HOSTINFOFILE, 0));
                                }
                            }
                        } while(0);
                        mainHandler().sendMessageDelayed(mainHandler().obtainMessage(
                                LFMT_HB_GAGENT, HB_GAGENT_UPGRADE_CHECK, 0), UPGRADE_CHECK_INTERVAL);
                        return true;
                    case HB_GAGENT_REGISTER_SUBDEV:
                        do {
                            LOGI("HB_GAGENT_REGISTER_SUBDEV\n");
                            if (!msg->obj)
                                return false;
#ifdef USE_SHARED_PTR
                            std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
#else
                            StringData *data = dynamic_cast<StringData*>(msg->obj);
#endif

                            std::vector<LFDeviceInfo> devs;
                            mainDB().queryBy(devs, data->getData());
                            if (devs.size() != 1)
                                return false;
                            if (devs[0].nEid.empty())
                                GAgent().registerSubdev(devs[0].nDid, devs[0].nName, devs[0].nKey);
                        } while(0);
                        return true;
                    case HB_GAGENT_SUBDEV_TOPOADD:
                        do {
                            LOGI("HB_GAGENT_SUBDEV_TOPOADD\n");
                            if (!msg->obj)
                                return false;
#ifdef USE_SHARED_PTR
                            std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
#else
                            StringData *data = dynamic_cast<StringData*>(msg->obj);
#endif
                            std::vector<LFDeviceInfo> devs;
                            mainDB().queryBy(devs, data->getData());
                            if (devs.size() != 1)
                                return false;
                            GAgent().topoAdd(devs[0].nEid, devs[0].nSecret, devs[0].nKey);
                        } while(0);
                        return true;
                    case HB_GAGENT_SUBDEV_TOPODEL:
                        break;
                    default:
                        break;
                }
                break;
            case LFMT_HB_UPGRADE:
                switch(msg->arg1) {
                    case HB_UPGRADE_HOSTINFOFILE:
                        do {
                            LOGI("LFMT_HB_UPGRADE.HB_UPGRADE_HOSTINFOFILE [%s]\n", gHostInfoFile.c_str());
                            std::ofstream of(gHostInfoFile.c_str());
                            if (of.is_open()) {
                                bool flg = false;
                                std::string hosteid;
                                std::string hostkey;
                                std::string sublist("\"sublist\":[");
                                std::vector<LFDeviceInfo> devs;
                                mainDB().queryBy(devs, LFDeviceInfo());
                                for (size_t i = 0; i < devs.size(); ++i) {
                                    if (devs[i].nType == DEVICE_TYPE_GATEWAY) {
                                        hosteid.assign(devs[i].nEid);
                                        hostkey.assign(devs[i].nKey);
                                        continue;
                                    }
                                    if (devs[i].nEid.empty())
                                        continue;
                                    if (flg) sublist.append(","); else flg = true;
                                    sublist.append("{\"elinkId\":\"").append(devs[i].nEid).append("\",");
                                    sublist.append("\"productKey\":\"").append(devs[i].nKey).append("\",");
                                    sublist.append("\"roomName\":\"").append(devs[i].nRoom).append("\"}");
                                }
                                sublist.append("]");
                                of << "\"devicelist\":{\"elinkId\":\"" << hosteid;
                                of << "\", \"productKey\": \"" << hostkey << "\", " << sublist << "}" << std::endl;
                                of.close();
                            }
                        } while(0);
                        break;
                    case HB_UPGRADE_BURNING:
                        do {
                            if (!msg->obj)
                                return false;
#ifdef USE_SHARED_PTR
                            std::shared_ptr<StringData> data(std::dynamic_pointer_cast<StringData>(msg->obj));
#else
                            StringData *data = dynamic_cast<StringData*>(msg->obj);
#endif
                            std::string newVersion = data->getData();
                            LOGI("LFMT_HB_UPGRADE.HB_UPGRADE_BURNING [%s]\n", newVersion.c_str());
                            LFOTAUpgrade(newVersion);
                        } while(0);
                        break;
                    default:
                        ;
                }
                return true;
            case MT_DB:
                if (msg->arg1 == DB_TABLE_UPDATE && msg->arg2 == TABLE_TYPE_DEVICE) {
                    // device table update need rewrite gHostInfoFile
                    mainHandler().sendMessage(
                        mainHandler().obtainMessage(LFMT_HB_UPGRADE, HB_UPGRADE_HOSTINFOFILE, 0));
                }
                break;
            default:
                ;
        }
        return false;
    }
};/*}}}*/

class InitThread: public ::UTILS::Thread {/*{{{*/
private:
    bool mEnableLogFile;
public:
    InitThread(bool enableLogFile = false) :  mEnableLogFile(enableLogFile) {}
    ~InitThread(){}
    void run() {
        mainHandler().mCallback = gStateBoot;

        /*-----------------------------------------------------------------
         *  SQLite Database
         *-----------------------------------------------------------------*/
        mainDB().init(gWorkspaceDir, 15000);


        /*-----------------------------------------------------------------
         *  Log Setting
         *-----------------------------------------------------------------*/
        std::vector<LFLogInfo> logs;
        mainDB().queryBy(logs, LFLogInfo());
        for (size_t i = 0; i < logs.size(); ++i)
            setModuleLogLevel(logs[i].nModule.c_str(), logs[i].nLevel);
        if (mEnableLogFile) {
            size_t size = 4096000;
            std::vector<LFGlobalInfo> infos;
            mainDB().queryBy(infos, LOG_FILESIZE_FIELD);
            if (infos.size() == 1)
                size = atoi(infos[0].nValue.c_str());
            if (size > 0) {
                LogPool::getInstance().attachFilter(
                    new LogFile(&mainHandler(), gWorkspaceDir.c_str(), size));
            }
        }

#ifdef ENABLE_MONITOR_TOOL
        /*-----------------------------------------------------------------
         *  Monitor Tool module init
         *-----------------------------------------------------------------*/
        printf("\nMonitor Init!\n");
        monitor().init(8192);
        monitor().start();
#endif

        /*-----------------------------------------------------------------
         *   Device Manager
         *-----------------------------------------------------------------*/
        deviceManager().getHostDeviceId(gLFHostInfo.nDid);
        deviceManager().setCallback(new LFDeviceCallback());

        std::vector<LFDeviceInfo> devs;
        mainDB().queryBy(devs, gLFHostInfo.nDid);
        if (devs.size() == 1)
            gLFHostInfo = devs[0];
        else {
            gLFHostInfo.nName.assign("HomeBrain");
            gLFHostInfo.nKey.assign("pk.33ef1b64b64");
            gLFHostInfo.nType = DEVICE_TYPE_GATEWAY;
            mainDB().updateOrInsert(gLFHostInfo);
        }

        /*-----------------------------------------------------------------
         *   Gateway Agent
         *-----------------------------------------------------------------*/
        GAgent().setRemoteCallback(new LFRemoteEventCallback());

        // TODO larfe using ethenet connect, so network is ok before start this program
        mainHandler().sendMessage(mainHandler().obtainMessage(MT_NETWORK,
                NETWORK_EVENT_CONNECT, NETWORK_CONNECT_SUCCESS));
        // TODO Just play a joke
        // mainHandler().sendEmptyMessageDelayed(MT_TIMER, 3000);
    }
};/*}}}*/

extern "C" int initMainThread();
extern "C" int mainThreadRun();

int main(int argc, char **argv)
{
    printf("###main###[%lu] [%s] [%s]\n", pthread_self(), LFBuildVersion(), LFBuildDatetime());

    int disablelogpool = 0;
    bool enablelogfile = false;
    int opt;
    while ((opt = getopt(argc, argv, "d:h:fn")) != -1) {
        switch (opt) {
            case 'd':
                gWorkspaceDir = optarg;
                break;
            case 'h':
                gHostInfoFile = optarg;
                break;
            case 'f':
                enablelogfile = true;
                break;
            case 'n':
                disablelogpool = 1;
                break;
            default:
                exit(1);
        }
    }
    if (chdir(gWorkspaceDir.c_str()) < 0)
        perror("chdir");

    // Log
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);
    if (disablelogpool)
        disableLogPool();

    // Main Message Queue
    initMainThread();

    gStateBoot = new BootStateCallback();
    gStateNormal = new NormalStateCallback();

    LFOTAInit();

    // Module Init Thread.
    InitThread init(enablelogfile); init.start();

    return mainThreadRun();
}
