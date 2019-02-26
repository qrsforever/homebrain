/***************************************************************************
 *  RuleEngineCore.cpp - Rule Clips Brain Impl
 *
 *  Created: 2018-06-13 09:28:22
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineCore.h"
#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "SysTime.h"
#include "RuleEngineLog.h"

#include <unistd.h>
#include <stdio.h>

#include <fstream>
#include <sstream>

#define SHOW_VALUE(item, tok) \
    do { \
        switch (item.type()) { \
        case TYPE_FLOAT: \
            RE_LOGI(tok"value  = float(%f)\n", item.as_float()); break; \
        case TYPE_INTEGER: \
            RE_LOGI(tok"value = int(%d)\n", item.as_integer()); break; \
        case TYPE_SYMBOL: \
        case TYPE_STRING: \
        case TYPE_INSTANCE_NAME: \
            RE_LOGI(tok"value = string(%s)\n", item.as_string().c_str()); break; \
        case TYPE_EXTERNAL_ADDRESS: \
        case TYPE_INSTANCE_ADDRESS: \
            RE_LOGI(tok"value = address(TODO)\n"); \
        } \
    } while(0)

#define SHOW_VALUES(items, tok) \
    do { \
        for (unsigned int i = 0; i < items.size(); ++i) \
            SHOW_VALUE(items[i], tok); \
    } while(0)

using namespace UTILS;
using namespace CLIPS;

namespace HB {

RuleEngineCore::RuleEngineCore(RuleEventHandler &handler, const char *id)
    : mEnv(0), mRouter(0), mID(id)
    , mHandler(handler)
{
    mLogLevel = getModuleLogLevel("rule-script");
}

RuleEngineCore::~RuleEngineCore()
{
    RE_LOGTT();
    mInses.clear();
    finalize();
}

void RuleEngineCore::setup(MsgPushPointer msgcall, InsPushPointer inscall, TxtPushPointer txtcall)
{
    if (!msgcall || !inscall || !txtcall)
        return;

    RE_LOGTT();
    Mutex::Autolock _l(&mEnvMutex);

    mEnv = new Environment();
    mRouter = new Router(*mEnv, mID, std::make_shared<CLIPSLogger>(mHandler));

    /* build defglobal version */
    char data[512] = { 0 };
    snprintf(data, 511,
        "(defglobal\n"
        "  ?*VERSION-MAJOR* = %u\n  ?*VERSION-MINOR* = %u\n  ?*VERSION-MICRO* = %u\n"
        "  ?*TYPE-TEM-FILE* = %u\n  ?*TYPE-CLS-FILE* = %u\n  ?*TYPE-RUL-FILE* = %u\n"
        "  ?*MSG-RULE-RESPONSE* = %d\n  ?*RUL-SUCCESS* = %d\n  ?*RUL-FAIL* = %d\n  ?*RUL-TIMEOUT* = %d\n"
        "  ?*MSG-RULE-RHS* = %d\n  ?*RHS-INS-NOT-FOUND* = %d\n  ?*RHS-NTF-WRONG-TYPE* = %d\n  ?*RHS-SEE-NOT-FOUND* = %d\n"
        ")\n",
        HB_VERSION_MAJOR, HB_VERSION_MINOR, HB_VERSION_MICRO,
        TYPE_TEM_FILE, TYPE_CLS_FILE, TYPE_RUL_FILE,
        MSG_RULE_RESPONSE, RUL_SUCCESS, RUL_FAIL, RUL_TIMEOUT,
        MSG_RULE_RHS, RHS_INS_NOT_FOUND, RHS_NTF_WRONG_TYPE, RHS_SEE_NOT_FOUND);
    mEnv->build(data);

    mEnv->setCallback(this);

    /* regist function for clips script */
    mEnv->add_function("get-debug-level", std::make_shared<Functor<int>>(this, &RuleEngineCore::onCallGetDebugLevel));
    mEnv->add_function("get-root-dir", std::make_shared<Functor<std::string>>(this, &RuleEngineCore::onCallGetRootDir));
    mEnv->add_function("get-files", std::make_shared<Functor<Values, int>>(this, &RuleEngineCore::onCallGetFiles));
    mEnv->add_function("now", std::make_shared<Functor<Values>>(this, &RuleEngineCore::onCallNow));
    mEnv->add_function("init-finished", std::make_shared<Functor<void>>(this, &RuleEngineCore::onCallInitFinished));

    mEnv->add_function("msg-push", msgcall);
    mEnv->add_function("ins-push", inscall);
    mEnv->add_function("txt-push", txtcall);
}

void RuleEngineCore::start(std::string &rootDir, GetFilesCallback callback)
{
    mRootDir = rootDir;
    mGetFilesCB = callback;
    mEnv->batch_evaluate(mRootDir + "/init.clp");
    assertRun("(init)");
}

long RuleEngineCore::assertRun(std::string assert)
{
    RE_LOGT("(%s)\n", assert.c_str());
    mEnv->assert_fact(assert);
    mEnv->refresh_agenda();
    return mEnv->run();
}

void RuleEngineCore::finalize()
{
    RE_LOGTT();

    assertRun("(finalize)");

    delete mRouter;
    delete mEnv;
    mRouter = 0;
    mEnv = 0;
}

void RuleEngineCore::getEnvBuildInfo(std::string &ver, std::string &user, std::string &dt)
{
    ver = mEnv->mCLPSVersion;
    user = mEnv->mCLPSCreator;
    dt = mEnv->mCLPSDateTime;
}

void RuleEngineCore::debug(int show)
{/*{{{*/
    RE_LOGI("===== BEGIN ===== \n");
    switch (show) {
        case DEBUG_SHOW_CLASSES:
            RE_LOGI(">> show classes:\n");
            assertRun("(show classes)");
            break;
        case DEBUG_SHOW_RULES:
            RE_LOGI(">> show rules:\n");
            assertRun("(show rules)");
            break;
        case DEBUG_SHOW_INSTANCES:
            RE_LOGI(">> show instances:\n");
            assertRun("(show instances)");
            {
                InstancesMap::iterator it;
                for (it = mInses.begin(); it != mInses.end(); ++it) {
                    RE_LOGI("\tInstance[%s]:\n", it->first.c_str());
                    it->second->send("print");
                }
            }
            break;
        case DEBUG_SHOW_FACTS:
            RE_LOGI(">> show facts:\n");
            assertRun("(show facts)");
            break;
        case DEBUG_SHOW_TEMPLATES:
            RE_LOGI(">> show templates:\n");
            assertRun("(show templates)");
            break;
        case DEBUG_SHOW_AGENDA:
            RE_LOGI(">> show agenda:\n");
            assertRun("(show agenda)");
            break;
        case DEBUG_SHOW_GLOBALS:
            RE_LOGI(">> show globals:\n");
            assertRun("(show globals)");
            break;
        case DEBUG_SHOW_MEMORY:
            RE_LOGI(">> show memory:\n");
            assertRun("(show memory)");
            break;
    }
    RE_LOGI("===== END ===== \n");
}/*}}}*/

void RuleEngineCore::setLogLevel(int level)
{/*{{{*/
    Global::pointer log = 0;
    log = mEnv->get_global("MAIN::LOG-LEVEL");
    if (log) {
        Value v(level);
        log->set_value(v);
        mLogLevel = level;
    }
}/*}}}*/

void RuleEngineCore::setWatch(bool watch, const std::string &item)
{
    if (watch)
        mEnv->watch(item);
    else
        mEnv->unwatch(item);
}

std::string RuleEngineCore::getRuleScript(const char *ruleId)
{
    RE_LOGI("getRuleScript rule [%s]\n", ruleId);
    if (ruleId) {
        Rule::pointer rule = mEnv->get_rule(ruleId);
        if (rule)
            return rule->formatted();
    }
    return std::string();
}

std::vector<std::string> RuleEngineCore::getRuleNames(const char *prefix)
{/*{{{*/
    Mutex::Autolock _l(&mEnvMutex);
    Module::pointer mod = mEnv->get_current_module();
    std::vector<std::string> rules = std::move(mEnv->get_rule_names(mod));
    if (!prefix)
        return std::move(rules);
    int len = strlen(prefix);
    std::vector<std::string> names;
    for (size_t i = 0; i < rules.size(); ++i) {
        if (0 == strncmp(rules[i].c_str(), prefix, len))
            names.push_back(rules[i]);
    }
    return std::move(names);
}/*}}}*/

std::vector<std::string> RuleEngineCore::getClassNames(const char *slotName)
{
    Mutex::Autolock _l(&mEnvMutex);

    std::vector<std::string> devices;
    Class::pointer cls = mEnv->get_class_list_head();
    for (; cls != 0; cls = cls->next()) {
        if (slotName && !cls->slot_exists(slotName, true))
            continue;
        devices.push_back(cls->name());
    }
    return std::move(devices);
}

std::vector<std::string> RuleEngineCore::getSlotNames(const char *clsName)
{
    if (!clsName)
        return std::vector<std::string>();

    Mutex::Autolock _l(&mEnvMutex);
    std::vector<std::string> slots;
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls) {
        std::vector<std::string> names = cls->slot_names(false);
        for (size_t i = 0; i < names.size(); ++i)
            slots.push_back(names[i]);
    }
    return std::move(slots);
}

std::vector<std::string> RuleEngineCore::getObjectNames(const char *clsName)
{
    if (!clsName)
        return std::vector<std::string>();

    Mutex::Autolock _l(&mEnvMutex);

    std::vector<std::string> names;
    Class::pointer cls;
    InstancesMap::iterator it = mInses.begin();
    for (; it != mInses.end(); ++it) {
        Class::pointer cls = it->second->getClass();
        if (cls && cls->name() == clsName)
            names.push_back(it->first);
    }
    return std::move(names);
}

std::string RuleEngineCore::getObjectValue(const char *insName, const char *slotName)
{
    if (!insName || !slotName)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);

    std::string res("");
    InstancesMap::iterator it = mInses.find(insName);
    if (it != mInses.end()) {
        Values rv;
        char value[64] = {0};
        rv = it->second->send("getData", slotName);
        if (rv.size() == 1) {
            switch (rv[0].type()) {
                case TYPE_FLOAT:
                    sprintf(value, "%f", rv[0].as_float());
                    break;
                case TYPE_INTEGER:
                    sprintf(value, "%ld", rv[0].as_integer());
                    break;
                case TYPE_SYMBOL:
                case TYPE_STRING:
                case TYPE_INSTANCE_NAME:
                    snprintf(value, 63, "%s", rv[0].as_string().c_str());
                    break;
                default:
                    break;
            }
            res = value;
        }
    }
    return std::move(res);
}

bool RuleEngineCore::handleTimer()
{
    Mutex::Autolock _l(&mEnvMutex);

    assertRun("(datetime (now))");

    /* false: again periodicly */
    return false;
}

void RuleEngineCore::handleError(const char *err)
{
    (void)err;
    Mutex::Autolock _l(&mEnvMutex);

    /* check halt */
    if (mEnv->get_halt_execution()) {
        RE_LOGE("OMG: halt execution, try restore!\n");
        mEnv->set_halt_execution(false);
    }
    if (mEnv->get_halt_rules()) {
        RE_LOGE("OMG: halt rules, try restore!\n");
        mEnv->set_halt_rules(false);
    }
    if (mEnv->get_evaluation_error()) {
        RE_LOGE("OMG: evaluation error, try restore!\n");
        mEnv->set_evaluation_error(false);
    }
}

std::string RuleEngineCore::handleClassSync(const char *clsName, const char *buildStr, bool fileClp)
{
    if (!clsName || !buildStr)
        return std::string("");

    RE_LOGD("(%s)\n%s\n", clsName, buildStr);

    Mutex::Autolock _l(&mEnvMutex);
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls) {
        /* TODO: cannot delete? */
        if (!cls->undefine()) {
            RE_LOGW("delete class[%s] fail! possible using in rule LHS.\n", clsName);
            return std::string("");
        }
    }
    if (!mEnv->build(buildStr)) {
        RE_LOGW("build class [%s] error!\n", clsName);
        return std::string("");
    }

    if (fileClp) {
        std::string path;
        path.append(mRootDir).append("/").append(CLSES_SEARCH_DIR);
        path.append(clsName).append(".clp");

        std::ofstream of(path);
        if (!of.is_open())
            return std::string("");
        of << buildStr << std::endl;
        of.close();
        return std::move(path);
    }
    return "";
}

std::string RuleEngineCore::handleRuleSync(const char *ruleId, const char *buildStr, bool fileClp)
{
    if (!ruleId || !buildStr)
        return std::string("");

    RE_LOGD("(%s)\n%s\n", ruleId, buildStr);

    Mutex::Autolock _l(&mEnvMutex);
    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (rule)
        rule->retract();
    if (!mEnv->build(buildStr)) {
        RE_LOGW("build rule [%s] error!\n", ruleId);
        return std::string("");
    }

    if (fileClp) {
        std::string path;
        path.append(mRootDir).append("/").append(RULES_SEARCH_DIR);
        path.append(ruleId).append(".clp");

        std::ofstream of(path);
        if (!of.is_open())
            return std::string("");
        of << buildStr << std::endl;
        of.close();
        return std::move(path);
    }
    return "";
}

bool RuleEngineCore::handleInstanceAdd(const char *insName, const char *clsName)
{
    if (!insName || !clsName)
        return false;

    RE_LOGD("(%s %s)\n", insName, clsName);

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it != mInses.end()) {
        RE_LOGW("Instance[%s] already exists!\n", insName);
        return false;
    }
    std::stringstream ss;
    ss << "(" << insName << " of " << clsName << ")";
    Instance::pointer ins = mEnv->make_instance(ss.str().c_str());
    if (!ins) {
        RE_LOGW("Make instance[%s] fail!\n", insName);
        return false;
    }
    mInses.insert(std::pair<std::string, Instance::pointer>(insName, ins));
    RE_LOGI("Make instance[%s] success!\n", insName);
    return true;
}

bool RuleEngineCore::handleInstanceDel(const char *insName)
{
    if (!insName)
        return false;

    RE_LOGD("(%s)\n", insName);

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        RE_LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    mInses.erase(it);
    return true;
}

bool RuleEngineCore::handleInstancePut(const char *insName, const char *slot, const char *value)
{
    if (!insName || !slot || !value)
        return false;

    RE_LOGT("(%s %s %s)\n", insName, slot, value);

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        RE_LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    std::string args;
    args.append(slot).append(" ").append(value);
    it->second->send("putData", args);

    assertRun("(datetime (now))");
    return true;
}

bool RuleEngineCore::deleteRule(const char *ruleId)
{
    RE_LOGI("(delete rule: %s)\n", ruleId);
    if (!ruleId)
        return false;
    Mutex::Autolock _l(&mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (rule)
        rule->retract();

    std::string path;
    path.append(mRootDir).append("/").append(RULES_SEARCH_DIR);
    path.append(ruleId).append(".clp");

    if (0 == access(path.c_str(), F_OK)) {
        if (0 == remove(path.c_str()))
            return true;
    }
    LOGW("remove:%s, error[%s]\n", strerror(errno));
    return false;
}

bool RuleEngineCore::enableRule(const char *ruleId, const char *buildStr)
{
    if (!ruleId)
        return false;

    RE_LOGD("enable rule [%s]\n", ruleId);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (rule)
        rule->retract();

    std::string constructStr;
    if (!buildStr) {
        std::string path;
        path.append(mRootDir).append("/").append(RULES_SEARCH_DIR);
        path.append(ruleId).append(".clp");

        std::stringstream scriptStr;
        std::ifstream in;
        in.open(path, std::ifstream::in);
        if (!in.is_open()) {
            RE_LOGW("open rule [%s] error!\n", path.c_str());
            return false;
        }
        scriptStr << in.rdbuf();
        in.close();
        constructStr = scriptStr.str();
    } else
        constructStr = buildStr;

    Mutex::Autolock _l(&mEnvMutex);

    if (!mEnv->build(constructStr)) {
        RE_LOGW("build rule [%s] error!\n", ruleId);
        return false;
    }
    return true;
}

bool RuleEngineCore::disableRule(const char *ruleId)
{
    if (!ruleId)
        return false;

    RE_LOGD("(disable rule: %s)\n", ruleId);

    Mutex::Autolock _l(&mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (!rule)
        return false;
    return rule->retract();
}

bool RuleEngineCore::refreshRule(const char *ruleId)
{
    RE_LOGI("refreshRule(%s)\n", ruleId);
    if (!ruleId)
        return false;
    Mutex::Autolock _l(mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (!rule)
        return false;
    rule->refresh();
    return true;
}

void RuleEngineCore::onCallClear()
{
    /* RE_LOGTT(); */
}

void RuleEngineCore::onCallReset(void)
{
    RE_LOGTT();
    mHandler.removeMessages(RET_REFRESH_TIMER);
    mHandler.sendEmptyMessage(RET_REFRESH_TIMER);
}

void RuleEngineCore::onPeriodic()
{
    /* RE_LOGTT(); */
}

void RuleEngineCore::onRuleFiring()
{
    /* RE_LOGTT(); */
}

int RuleEngineCore::onCallGetDebugLevel()
{
    return mLogLevel;
}

std::string RuleEngineCore::onCallGetRootDir()
{
    return mRootDir;
}

Values RuleEngineCore::onCallGetFiles(int fileType)
{
    Values rv;
    if (mGetFilesCB) {
        std::vector<std::string> files;
        files = mGetFilesCB(fileType);
        for (size_t i = 0; i < files.size(); ++i) {
            RE_LOGI("clp file: [%s]\n", files[i].c_str());
            rv.push_back(files[i]);
        }
    }
    return rv;
}

Values RuleEngineCore::onCallNow()
{
    SysTime::DateTime dt;
    SysTime::GetDateTime(&dt);
    Values rv;
    rv.push_back(SysTime::GetMSecs()); /* clock time */
    rv.push_back(dt.mYear);
    rv.push_back(dt.mMonth);
    rv.push_back(dt.mDay);
    rv.push_back(dt.mHour);
    rv.push_back(dt.mMinute);
    rv.push_back(dt.mSecond);
    rv.push_back(dt.mDayOfWeek); /* week day */
    return rv;
}

void RuleEngineCore::onCallInitFinished()
{

}

} /* namespace HB */
