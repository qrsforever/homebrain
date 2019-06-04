/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeSwitch.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeSwitch";

void KonkeSwitch::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "power: " << m_power);
}

void KonkeSwitch::get(std::string &info)
{
    info = m_power?"ON":"OFF";
    DEBUG_PRINT("::get: " <<info);
}
void KonkeSwitch::put(std::string arg)
{
   sendMessage("SWITCH", arg.c_str());
}

void KonkeSwitch::update(std::string arg, std::string opcode)
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
