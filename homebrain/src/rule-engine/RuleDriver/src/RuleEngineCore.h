/***************************************************************************
 *  RuleEngineCore.h - Rule Clips Brain
 *
 *  Created: 2018-06-13 09:26:42
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineCore_H__
#define __RuleEngineCore_H__

#include "Environment.h"
#include "Router.h"
#include "Mutex.h"
#include <map>
#include <vector>

#ifdef __cplusplus

#define HB_VERSION_MAJOR 1
#define HB_VERSION_MINOR 0
#define HB_VERSION_MICRO 0

namespace HB {

class RuleEventHandler;

typedef std::map<std::string, CLIPS::Instance::pointer> InstancesMap;
typedef std::map<std::string, CLIPS::Rule::pointer> RulesMap;

typedef CLIPS::Functor<bool, int, int, std::string, std::string> MsgPushCall;
typedef CLIPS::Functor<bool, std::string, std::string, std::string> InsPushCall;
typedef CLIPS::Functor<bool, std::string, std::string, std::string> TxtPushCall;

typedef std::shared_ptr<MsgPushCall> MsgPushPointer;
typedef std::shared_ptr<InsPushCall> InsPushPointer;
typedef std::shared_ptr<TxtPushCall> TxtPushPointer;

typedef std::function<std::vector<std::string>(int type)> GetFilesCallback;

class RuleEngineCore : public CLIPS::EnvironmentCallback {
public:
    typedef std::shared_ptr<RuleEngineCore> pointer;
    RuleEngineCore(RuleEventHandler &handler, const char *id);
    ~RuleEngineCore();

    CLIPS::Environment& driver() { return *mEnv; }
    void setup(MsgPushPointer msgcall, InsPushPointer inscall, TxtPushPointer txtcall);
    void start(std::string &rootDir, GetFilesCallback callback);
    void finalize();
    long assertRun(std::string assert);

    void reset();
    bool handleTimer();
    void handleError(const char *err);

    std::string handleClassSync(const char *clsName, const char *buildStr, bool fileClp = false);
    std::string handleRuleSync(const char *ruleId, const char *buildStr, bool fileClp = false);

    /* object */
    bool handleInstanceAdd(const char *insName, const char *clsName);
    bool handleInstanceDel(const char *insName);
    bool handleInstancePut(const char *insName, const char *slot, const char *value);

    /* rule */
    bool refreshRule(const char *ruleId);
    bool deleteRule(const char *ruleId);
    bool enableRule(const char *ruleId, const char *buildStr = 0);
    bool disableRule(const char *ruleId);

    void debug(int show);
    void setWatch(bool watch, const std::string &item);

    /* log */
    void setLogLevel(int level);
    int getLogLevel() { return mLogLevel; }

    /* monitor */
    std::string getRuleScript(const char *ruleId);
    std::vector<std::string> getRuleNames(const char *prefix);
    std::vector<std::string> getClassNames(const char *slotName);
    std::vector<std::string> getSlotNames(const char *clsName);
    std::vector<std::string> getObjectNames(const char *clsName);
    std::string getObjectValue(const char *insName, const char *slotName);

    void getEnvBuildInfo(std::string &ver, std::string &user, std::string &dt);

private:

    /* inherit from EnvironmentCallback */
    void onCallClear();
    void onCallReset();
    void onPeriodic();
    void onRuleFiring();

    int onCallGetDebugLevel();
    std::string onCallGetRootDir();
    CLIPS::Values onCallGetFiles(int fileType);
    CLIPS::Values onCallNow();
    void onCallInitFinished();

private:
    UTILS::Mutex mEnvMutex;
    CLIPS::Environment *mEnv;
    CLIPS::Router *mRouter;
    std::string mID;
    RuleEventHandler &mHandler;
    InstancesMap mInses;
    std::string mRootDir;
    GetFilesCallback mGetFilesCB;
    int mLogLevel;
}; /* class RuleEngineCore */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineCore_H__ */
