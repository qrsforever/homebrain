/***************************************************************************
 *  LFLFMonitorTool.h -
 *
 *  Created: 2019-07-15 10:54:02
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LFMonitorTool_H__
#define __LFMonitorTool_H__

#include "Thread.h"

#include <vector>
#include <string>
#include <map>
#include <functional>

#ifdef __cplusplus

namespace HB {

class LFMonitorTool : public ::UTILS::Thread {
public:
    typedef std::function<std::string(const char *prams)> CommandCall_t;
    typedef std::map<std::string, CommandCall_t>::iterator CommandIter_t;
    LFMonitorTool();
    ~LFMonitorTool();

    int init(int port = 8192);
    void registCommands();
    void run();
    void finalize();

    bool doRequest(int sockfd);

    int addClient(int sockfd);
    int delClient(int sockfd);

private:
    void _Insert(const std::string &name, CommandCall_t cmd);

    /* Commands */
    static std::string _SetModuleLogLevel(const char *params);
    static std::string _GetModuleLogLevel(const char *params);
    static std::string _GetModulesName(const char *params);
    static std::string _StartUDPLog(const char *params);
    static std::string _StopUDPLog(const char *params);
    static std::string _SetLogFileSize(const char *params);
    static std::string _GetLogFileSize(const char *params);
    static std::string _SetFtpLog(const char *params);
    static std::string _GetFtpLogParams(const char *params);

private:
    int mServerPort;
    int mRefreshFds[2];
    bool mQuitFlag;
    std::vector<int> mClientSockets;
    std::map<std::string, CommandCall_t> mCommands;
}; /* class LFMonitorTool */

inline void LFMonitorTool::_Insert(const std::string &name, CommandCall_t cmd)
{
    mCommands.insert(std::pair<std::string, CommandCall_t>(name, cmd));
}

LFMonitorTool& monitor();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __LFMonitorTool_H__ */
