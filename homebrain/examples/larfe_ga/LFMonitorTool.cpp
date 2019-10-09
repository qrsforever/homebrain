/***************************************************************************
 *  LFMonitorTool.cpp -
 *
 *  Created: 2019-07-15 10:54:26
 *
 *  Copyright QRS
 ****************************************************************************/

#include "LFMonitorTool.h"
#include "Log.h"
#include "LogUDP.h"
#include "LogPool.h"
#include "DBTables.h"
#include "LFDatabase.h"
#include "LFHelper.h"
#include "Common.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TOCKEN "^"

using namespace UTILS;

namespace HB {

static LFMonitorTool *gMonitor = 0;
static LogUDP *gLogUDP = 0;

LFMonitorTool::LFMonitorTool()
    : mServerPort(8192)
    , mQuitFlag(false)
{
}

LFMonitorTool::~LFMonitorTool()
{
}

int LFMonitorTool::init(int port)
{/*{{{*/
    LOGD("LFMonitorTool port[%d]\n", port);
    mServerPort = port;
    mRefreshFds[0] = -1;
    mRefreshFds[1] = -1;
    return 0;
}/*}}}*/

void LFMonitorTool::finalize()
{/*{{{*/
    mQuitFlag = true;
    int nouse = 1;
    if (mRefreshFds[1] > 0) {
        int ret = write(mRefreshFds[1], &nouse, sizeof(nouse));
        (void)ret;
    }
}/*}}}*/

int LFMonitorTool::addClient(int sockfd)
{/*{{{*/
    LOGI("add Client: %d\n", sockfd);
    char ok[8] = "success";
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    send(sockfd, ok, 7, 0);
    mClientSockets.push_back(sockfd);
    /* for auto close socket after 300s */
    /* mainHandler().removeMessages(MT_MONITOR, MONITOR_CLOSE_CLIENT, sockfd);
     * mainHandler().sendMessageDelayed(
     *     mainHandler().obtainMessage(MT_MONITOR, MONITOR_CLOSE_CLIENT, sockfd), 300000);  */
    return 0;
}/*}}}*/

int LFMonitorTool::delClient(int sockfd)
{/*{{{*/
    LOGI("del Client: %d\n", sockfd);
    /* mainHandler().removeMessages(MT_MONITOR, MONITOR_CLOSE_CLIENT, sockfd); */
    for (size_t i = 0; i < mClientSockets.size(); ++i) {
        if (mClientSockets[i] == sockfd)
            mClientSockets.erase(mClientSockets.begin() + i);
    }
    close(sockfd);
    return 0;
}/*}}}*/

void LFMonitorTool::run()
{/*{{{*/
    LOGD("LFMonitorTool listen thread:[%u]\n", id());

    fd_set rset;
    socklen_t sock_size;
    struct sockaddr_in cli_addr;
    struct sockaddr_in svr_addr;
    int cli_sockfd = -1;
    int svr_sockfd = -1;
    int maxFd = -1;
    int retnum = 0;
    int opt = 1;

    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(mServerPort);

    if ((svr_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        LOGE("socket error port[%d]\n", mServerPort);
        return;
    }
    setsockopt(svr_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (::bind(svr_sockfd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr)) < 0) {
        LOGE("bind error [%s]\n", strerror(errno));
        return;
    }
    fcntl(svr_sockfd, F_SETFL, O_NONBLOCK);

    registCommands();

    int ret = pipe(mRefreshFds);
    (void)ret;
    while (!mQuitFlag) {
        FD_ZERO(&rset);
        FD_SET(mRefreshFds[0], &rset);
        FD_SET(svr_sockfd, &rset);
        maxFd = svr_sockfd > mRefreshFds[0] ? svr_sockfd : mRefreshFds[0];
        for (size_t i = 0; i < mClientSockets.size(); ++i) {
            if (mClientSockets[i] > maxFd)
                maxFd = mClientSockets[i];
            FD_SET(mClientSockets[i], &rset);
        }
        LOGI("maxFd = %d cliens[%d]\n", maxFd, mClientSockets.size());
        retnum = select(maxFd + 1, &rset, 0, 0, 0);
        if (-1 == retnum) {
            if (EINTR == errno)
                continue;
            return;
        }
        if (FD_ISSET(mRefreshFds[0], &rset)) {
            ret = read(mRefreshFds[0], &opt, sizeof(opt));
            (void)ret;
            continue;
        }
        if (FD_ISSET(svr_sockfd, &rset)) {
            if (listen(svr_sockfd, 5) < 0) {
                LOGE("listen error[%s]!\n", strerror(errno));
                continue;
            }
            memset(&cli_addr, 0, sizeof(struct sockaddr_in));
            sock_size = 1;
            cli_sockfd = accept(svr_sockfd, (struct sockaddr *)&cli_addr, &sock_size);
            LOGI("cli_sockfd = %d\n", cli_sockfd);
            if (cli_sockfd > 0)
                addClient(cli_sockfd);
            else
                LOGE("accept error:%s\n", strerror(errno));
        } else {
            for (size_t i = 0; i < mClientSockets.size(); ++i) {
                if (FD_ISSET(mClientSockets[i], &rset))
                    if (!doRequest(mClientSockets[i]))
                        send(mClientSockets[i], "-1", 2, 0);
            }
        }
    }
    for (size_t i = 0; i < mClientSockets.size(); ++i) {
        send(mClientSockets[i], "-2", 2, 0);
        delClient(mClientSockets[i]);
    }
    close(svr_sockfd);
    LOGD("Monitor Thread Quit.\n");
}/*}}}*/

void LFMonitorTool::registCommands()
{/*{{{*/
    _Insert("setModuleLogLevel",    &LFMonitorTool::_SetModuleLogLevel);
    _Insert("getModuleLogLevel",    &LFMonitorTool::_GetModuleLogLevel);
    _Insert("getModulesName",       &LFMonitorTool::_GetModulesName);
    _Insert("startUDPLog",          &LFMonitorTool::_StartUDPLog);
    _Insert("stopUDPLog",           &LFMonitorTool::_StopUDPLog);
    _Insert("setLogFileSize",       &LFMonitorTool::_SetLogFileSize);
    _Insert("getLogFileSize",       &LFMonitorTool::_GetLogFileSize);
    _Insert("setFtpLog",            &LFMonitorTool::_SetFtpLog);
    _Insert("getFtpLogParams",      &LFMonitorTool::_GetFtpLogParams);

}/*}}}*/

std::string LFMonitorTool::_SetModuleLogLevel(const char *params)
{/*{{{*/
    std::vector<std::string> ps = stringSplit(params, TOCKEN);
    if (ps.size() != 2) {
        LOGE("params[%d] format error!\n", ps.size());
        return "-1";
    }
    int l = atoi(ps[1].c_str());

    LFLogInfo log;
    log.nModule = ps[0];
    log.nLevel = l;
    mainDB().updateOrInsert(log);

    return int2String(setModuleLogLevel(ps[0].c_str(), l));
}/*}}}*/

std::string LFMonitorTool::_GetModuleLogLevel(const char *params)
{/*{{{*/
    std::vector<std::string> ps = stringSplit(params, TOCKEN);
    if (ps.size() != 1) {
        LOGE("params[%d] format error!\n", ps.size());
        return "-1";
    }
    return int2String(getModuleLogLevel(ps[0].c_str()));
}/*}}}*/

std::string LFMonitorTool::_GetModulesName(const char *params)
{/*{{{*/
    (void)params;
    char names[128];
    return getModuleLogNames(names, 127);
}/*}}}*/

std::string LFMonitorTool::_StartUDPLog(const char *params)
{/*{{{*/
    if (gLogUDP)
        return "-1";
    std::vector<std::string> ps = stringSplit(params, TOCKEN);
    if (ps.size() != 2) {
        LOGE("params[%d] format error!\n", ps.size());
        return "-1";
    }
    LOGD("startUDPLog(%s, %d)\n", ps[0].c_str(), atoi(ps[1].c_str()));
    gLogUDP = new LogUDP(ps[0].c_str(), atoi(ps[1].c_str()));
    LogPool::getInstance().attachFilter(gLogUDP);
    LFDumpInfo();
    return "0";
}/*}}}*/

std::string LFMonitorTool::_StopUDPLog(const char *params)
{/*{{{*/
    (void)params;
    if (gLogUDP) {
        LogPool::getInstance().detachFilter(gLogUDP);
        delete (gLogUDP);
        gLogUDP = 0;
    }
    return "0";
}/*}}}*/

std::string LFMonitorTool::_SetLogFileSize(const char *params)
{/*{{{*/
    if (!params)
        return "-1";
    LFGlobalInfo info;
    info.nName.assign(LOG_FILESIZE_FIELD);
    info.nValue.assign(params);
    mainDB().updateOrInsert(info);
    return "0";
}/*}}}*/

std::string LFMonitorTool::_GetLogFileSize(const char *params)
{/*{{{*/
    (void)params;
    std::vector<LFGlobalInfo> infos;
    mainDB().queryBy(infos, LOG_FILESIZE_FIELD);
    if (infos.size() != 1)
        return "-1";
    return infos[0].nValue;
}/*}}}*/

std::string LFMonitorTool::_SetFtpLog(const char *params)
{/*{{{*/
    std::vector<std::string> ps = stringSplit(params, TOCKEN);
    if (ps.size() != 3) {
        mainDB().deleteBy(LFGlobalInfo(LOG_FTP_FIELD));
        return "0";
    }
    LFGlobalInfo info;
    info.nName.assign(LOG_FTP_FIELD);
    info.nValue.assign(params);
    mainDB().updateOrInsert(info);
    return "0";
}/*}}}*/

std::string LFMonitorTool::_GetFtpLogParams(const char *params)
{/*{{{*/
    (void)params;
    std::vector<LFGlobalInfo> infos;
    mainDB().queryBy(infos, LOG_FTP_FIELD);
    if (infos.size() != 1)
        return "-1";
    return infos[0].nValue;
}/*}}}*/

bool LFMonitorTool::doRequest(int sockfd)
{/*{{{*/
    char buff[4096] = { 0 };
    if (recv(sockfd, buff, 4095, 0) <= 0) {
        LOGE("recv error[%s]!\n", strerror(errno));
        delClient(sockfd);
        return true;
    }
    std::string res("-1");
    char *cmd = strtok(buff, TOCKEN);
    if (!cmd) {
        LOGE("command[%s] format error!\n", buff);
        return false;
    }
    CommandIter_t it = mCommands.find(cmd);
    if (it == mCommands.end()) {
        if (!strncmp(cmd, "quit", 4)) {
            delClient(sockfd);
            return true;
        } else {
            LOGW("Unkown command[%s]\n", cmd);
            return -1;
        }
    } else
        res = it->second(buff + strlen(cmd) + 1);

    LOGD("command[%s]: [%s]\n", buff, res.c_str());
    if (!res.empty())
        return send(sockfd, res.c_str(), res.size(), 0);
    return false;
}/*}}}*/

LFMonitorTool& monitor()
{
    if (0 == gMonitor)
         gMonitor = new LFMonitorTool();
    return *gMonitor;
}

} /* namespace HB */
