/***************************************************************************
 *  Log.cpp - Log Impl
 *
 *  Created: 2018-06-04 13:44:55
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"
#include "LogPool.h"
#include "LogSource.h"
#include "LogConsole.h"
#include "LogThread.h"
#include "RingBuffer.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE (128*1024)
#endif

/* TODO */
static uint8_t *g_logBuffer = 0;
static UTILS::RingBuffer *g_ringBuffer = 0;
static UTILS::LogSource *g_logSource = 0;
#ifndef __ANDROID__
static UTILS::LogConsole *g_logConsole = 0;
#endif
static UTILS::LogThread *g_logThread = 0;

static struct LogModule* g_moduleListHead = 0;

LogModule::LogModule(const char* name, int& level)
	: m_name(name)
	, m_level(level)
{
    m_next = g_moduleListHead;
    g_moduleListHead = this;
}

LogModule::~LogModule()
{
}

int g_logLevel = LOG_LEVEL_WARNING;
bool g_logPoolEnable = true;

static LogModule DefaultModule("default", g_logLevel);

static void _LogVerbose(const char *file, int line, const char *function, int level, const char *fmt, va_list args)
{
    static char buffer[2048] = { 0 };
    const char* pFile = strrchr(file, '/');
    if (pFile)
        pFile = pFile + 1;
    else
        pFile = file;
#ifdef __ANDROID__
    int len = snprintf(buffer, 2047, "%u | %s:%d | %s | %d | ", (unsigned int)pthread_self(), pFile, line, function, level);
    vsnprintf(buffer + len, 2047 - len, fmt, args);
    __android_log_write(ANDROID_LOG_VERBOSE, HB_TAG, buffer);
#else
    vsnprintf(buffer, 2047, fmt, args);
    printf("%u | %s:%d | %s | %d | %s",  (unsigned int)pthread_self(), pFile, line, function, level, buffer);
#endif
}

extern "C"
void logInit() /* called in LogThread */
{
    g_logBuffer = (uint8_t*)malloc(LOG_BUFFER_SIZE);
    g_ringBuffer = new UTILS::RingBuffer(g_logBuffer, LOG_BUFFER_SIZE);

    UTILS::LogPool *logPool = &UTILS::LogPool::getInstance();
    logPool->setBuffer(g_ringBuffer);

#ifndef __ANDROID__
    g_logConsole = new UTILS::LogConsole();
    logPool->attachFilter(g_logConsole);
#endif

    g_logSource = &UTILS::LogSource::getInstance();
    g_logSource->attachSink(logPool);
}

extern "C"
void logVerbose(const char *file, int line, const char *function, int level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (g_logPoolEnable) {
#ifdef __ANDROID__
        _LogVerbose(file, line, function, level, fmt, args);
#endif
        g_logSource->logVerbose(file, line, function, level, fmt, args);
    } else
        _LogVerbose(file, line, function, level, fmt, args);
    va_end(args);
}

extern "C"
void setLogLevel(int level)
{
    g_logLevel = level;
}

extern "C"
int getLogLevel()
{
    return g_logLevel;
}

extern "C"
int setModuleLogLevel(const char *name, int level)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name)) {
            int old = module->m_level;
            module->m_level = level;
            return old;
        }
        module = module->m_next;
    }
    return -1;
}

extern "C"
int getModuleLogLevel(const char *name)
{
    struct LogModule* module = g_moduleListHead;

    while (module) {
        if (!strcmp(module->m_name, name))
            return module->m_level;
        module = module->m_next;
    }
    return -1;
}

extern "C"
void disableLogPool()
{
    g_logPoolEnable = false;
}

extern "C"
void enableLogPool()
{
    g_logPoolEnable = true;
}

extern "C"
void finishLogPool()
{
    // UTILS::LogPool *logPool = &UTILS::LogPool::getInstance();
    // logPool->onEnd();
}

extern "C"
char* getModuleLogNames(char* names, int len)
{
    if (!names || len <= 0)
        return names;

    int sumlen = 0;
    struct LogModule* module = g_moduleListHead;
    while (module) {
        if (len > sumlen)
            sumlen += snprintf(names + sumlen, len - sumlen, "%s^", module->m_name);
        module = module->m_next;
    }
    if (sumlen > 0)
        names[sumlen-1] = 0;
    return names;
}

extern "C"
void initLogThread()
{
    if (0 == g_logThread) {
        g_logThread = new UTILS::LogThread();
        g_logThread->start();
    }
}

