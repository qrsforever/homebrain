/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeSOSAlarm.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeSOSAlarm";

void KonkeSOSAlarm::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "power: " << m_power);
}

void KonkeSOSAlarm::get(std::string &info)
{
    info = m_power?"ON":"OFF";
    DEBUG_PRINT("::get: " <<info);
}
void KonkeSOSAlarm::put(std::string arg)
{
    sendMessage("SWITCH", "CLEAR_SOS_ALARM");
}

void KonkeSOSAlarm::update(std::string arg, std::string opcode)
{
    bool old = m_power;
    if (!strcmp(arg.c_str(), "ON")) {
	    m_power = true;
    }else {
	    m_power = false;
    }
    if (old != m_power)
        notifyObservers();
    DEBUG_PRINT("::update: " <<m_power);
}
