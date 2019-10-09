/***************************************************************************
 *  HBHelper.cpp - impl
 *
 *  Created: 2018-08-06 17:54:29
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBHelper.h"
#include "Log.h"
#include "CloudDataChannel.h"
#include "DeviceProfileTable.h"
#include "HBDatabase.h"
#include "ClassPayload.h"
#include "Common.h"
#include "IPCSystem.h"

#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/prctl.h>
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <fstream>
#include <sstream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static int gChnnl_w = -1;
static int gChnnl_r = -1;
static std::map<std::string, int> gBridges;

namespace HB {

#ifdef ENABLE_LE_CLOUD
extern "C" HBCloudManager& getCloudManager();
HBCloudManager& cloudManager()
{
    return getCloudManager();
}
#else
HBCloudManager& cloudManager()
{
    static HBCloudManager s_default;
    return s_default;
}
#endif

HBDeviceManager& deviceManager()
{
    return *(HBDeviceManager::GetInstance());
}

void parseDeviceProfileJson(const char *filepath)
{/*{{{*/
    if (!filepath)
        return;

    std::stringstream jsonDoc;
    std::ifstream in;
    in.open(filepath, std::ifstream::in);
    if (!in.is_open()) {
        LOGE("open file[%s] error!\n", filepath);
        return;
    }
    jsonDoc << in.rdbuf();
    in.close();

    rapidjson::Document doc;
    std::string iconId("ic_default_device");
    std::string superType("DEVICE");

    doc.Parse<0>(jsonDoc.str().c_str());
    if (doc.HasParseError()) {
        rapidjson::ParseErrorCode code = doc.GetParseError();
        LOGE("rapidjson parse[%s] error[%d]\n", filepath, code);
        return;
    }

    if (!doc.HasMember("version") && !doc.HasMember("profile")
        && !doc.HasMember("devicetype") && !doc.HasMember("devicename")) {
        LOGE("rapidjson parse[%s] error!\n", filepath);
        return;
    }

    rapidjson::Value &devicetype = doc["devicetype"];

    rapidjson::Value &devicename = doc["devicename"];
    rapidjson::Value &version = doc["version"];
    rapidjson::Value &manufacture = doc["manufacture"];

    if (doc.HasMember("iconid")) {
        rapidjson::Value &iconid = doc["iconid"];
        iconId.assign(iconid.GetString());
    }

    if (doc.HasMember("supertype")) {
        rapidjson::Value &super = doc["supertype"];
        superType.assign(super.GetString());
    }

    rapidjson::Value &profile = doc["profile"];
    if (!profile.IsObject()) {
        LOGE("rapidjson parse[%s] profile error!\n", filepath);
        return;
    }

    DeviceTableInfo info;
    info.nDeviceType = devicetype.GetString();

    std::vector<DeviceTableInfo> prfs;
    mainDB().queryBy(prfs, info);
    if (prfs.size() == 1) {
        if (prfs[0].nDeviceVer == version.GetString()) {
            return;
        }
    }
    info.nDeviceVer = version.GetString();

    std::shared_ptr<ClassPayload> payload = std::make_shared<ClassPayload>(info.nDeviceType, superType, info.nDeviceVer);
    if (!ElinkCloudDataChannel::parseProfile(profile, payload)) {
        LOGE("rapidjson parse[%s] profile error!\n", filepath);
        return;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    profile.Accept(writer);

    info.nDeviceName = devicename.GetString();
    info.nDeviceManu = manufacture.GetString();
    info.nIconId = iconId;
    info.nSuperType = superType;
    info.nScriptData = payload->toString();
    info.nJsonData = buffer.GetString();

    mainDB().updateOrInsert(info);
}/*}}}*/

void deviceProfileCheckAndUpdate(const char *path)
{/*{{{*/
    DIR *dirp = 0;
    struct dirent *direntp = 0;

    DIR *dirTypep = 0;
    struct dirent *direntTypep = 0;

    char catpath[256] = { 0 };
    char *ptr = 0;

    dirp = opendir(path);
    if (dirp == 0)
        return;

    while ((direntp = readdir(dirp)) != 0) {
        if (0 == strcmp(direntp->d_name, "."))
            continue;
        if (0 == strcmp(direntp->d_name, ".."))
            continue;
        if (direntp->d_type == DT_REG) {
            /* possible global json */
            ptr = strrchr(direntp->d_name, '.');
            if (ptr == 0)
                continue;
            if (0 == strcmp(ptr, ".json")) {
                snprintf(catpath, 255, "%s/%s", path, direntp->d_name);
                parseDeviceProfileJson(catpath);
            }
        } else if (direntp->d_type == DT_DIR) {
            /* manufacture dir */
            snprintf(catpath, 255, "%s/%s", path, direntp->d_name);
            dirTypep = opendir(catpath);
            if (dirTypep == 0)
                continue;
            while ((direntTypep = readdir(dirTypep)) != NULL){
                if (direntTypep->d_type == DT_REG) {
                    ptr = strrchr(direntTypep->d_name, '.');
                    if (ptr == 0)
                        continue;
                    if (0 == strcmp(ptr, ".json")) {
                        snprintf(catpath, 255, "%s/%s/%s", path, direntp->d_name, direntTypep->d_name);
                        parseDeviceProfileJson(catpath);
                    }
                }
            }
            closedir(dirTypep);
        }
    }
    closedir(dirp);
}/*}}}*/

int lightSystemCall(int what, int argc, char *args[])
{/*{{{*/
#if defined(__ANDROID__)
    if (gChnnl_w < 0) {
        if ((gChnnl_w = open(HB_WRITE_KEY, O_RDWR)) < 0) {
            LOGE("open write fifo error[%s]", strerror(errno));
            return -1;
        }
    }

    if (gChnnl_r < 0) {
        if ((gChnnl_r = open(HB_READ_KEY, O_RDWR)) < 0) {
            LOGE("open read fifo error[%s]", strerror(errno));
            return -1;
        }
    }
#else
    if (gChnnl_w < 0) {
        if ((gChnnl_w = msgget(HB_WRITE_KEY, IPC_CREAT|0660)) < 0) {
            LOGE("open msgget write error[%s]", strerror(errno));
            return -1;
        }
    }
    if (gChnnl_r < 0) {
        if ((gChnnl_r = msgget(HB_READ_KEY, IPC_CREAT|0660)) < 0) {
            LOGE("open msgget read error[%s]", strerror(errno));
            return -1;
        }
    }
#endif
    IPCMessage_t msg;
    memset(&msg, 0, sizeof(IPCMessage_t));
    msg.what = what;
    msg.u.argc = argc;
    for (int i = 0; i < argc; ++i)
        strncpy(msg.args[i], args[i], ARG_SIZE - 1);
#if defined(__ANDROID__)
    if (write(gChnnl_w, &msg, sizeof(IPCMessage_t)) < 0) {
#else
    if (msgsnd(gChnnl_w, &msg, sizeof(IPCMessage_t) - sizeof(long), 0) < 0) {
#endif
        perror("msgsnd/write");
        return -1;
    }
    msg.u.result = -1;
#if defined(__ANDROID__)
    read(gChnnl_r, &msg, sizeof(IPCMessage_t));
#else
    msgrcv(gChnnl_r, &msg, sizeof(IPCMessage_t) - sizeof(long), what, 0);
#endif
    return msg.u.result;
}/*}}}*/

int startBridge(const char* bridgeType, const char *bridgeId, const char *accessKey, const char *ip)
{/*{{{*/
    if (!bridgeType || !bridgeId || !accessKey)
        return -1;

    killBridge(bridgeId);

    int pid = -1;;
    if (!strncmp(bridgeType, "konke", 5)) {
        char *args[] = {(char*)"kk_bridge", (char*)bridgeId, (char*)accessKey};
        pid = lightSystemCall(MSG_SYSTEM_CALL, 3, args);
    } else if (!strncmp(bridgeType, "hue", 3)) {
        char *args[] = {(char*)"hue_bridge", (char*)"-u", (char*)bridgeId,
            (char*)"-k", (char*)accessKey, (char*)"-s", (char*)ip
        };
        pid = lightSystemCall(MSG_SYSTEM_CALL, 7, args);
    }
    if (pid > 0) {
        gBridges[bridgeId] = pid;
        return pid;
    }
    return -1;
}/*}}}*/

int killBridge(const char *bridgeId)
{/*{{{*/
    auto it = gBridges.find(bridgeId);
    if (it != gBridges.end())
        kill(it->second, SIGKILL);
    return 0;
}/*}}}*/

} /* namespace HB */
