/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeSmartplug.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeSmartplug";

void KonkeSmartplug::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "status: " << m_status);
}

void KonkeSmartplug::get(std::string &info)
{
    info = m_status?"ON":"OFF";
    DEBUG_PRINT("::get: " <<info);
}
void KonkeSmartplug::put(std::string arg)
{
    if (!strcmp(arg.c_str(), "ON")) {
       sendMessage("SWITCH", "ON");
    }else if (!strcmp(arg.c_str(), "OFF")) {
       sendMessage("SWITCH", "OFF");
    }else if (!strcmp(arg.c_str(), "GET_VOLTAGE")) {
       sendMessage("GET_VOLTAGE", "*");
    }else if (!strcmp(arg.c_str(), "GET_ELECTRICAL")) {
       sendMessage("GET_ELECTRICAL", "*");
    }else if (!strcmp(arg.c_str(), "GET_DEV_POWER")) {
       sendMessage("GET_DEV_POWER", "*");
    }else {
    }
}

void KonkeSmartplug::update(std::string arg, std::string opcode)
{
    bool old = m_status;
    if (!strcmp(opcode.c_str(), "SWITCH")) {
    if (!strcmp(arg.c_str(), "ON")) {
	    m_status = true;
    }else if (!strcmp(arg.c_str(), "OFF")) {
	    m_status = false;
    }
    }else {
	rapidjson::Document doc;
        doc.Parse<0>(arg.c_str());
	if (!strcmp(opcode.c_str(), "GET_ELECTRICAL")) {
             std::string s_elec = doc["electrical"].GetString();
             m_electrical = atoi(s_elec.c_str());
	}else if (!strcmp(opcode.c_str(), "GET_VOLTAGE")) {
             std::string s_vol = doc["voltage"].GetString();
             m_voltage = atoi(s_vol.c_str());
	}else if (!strcmp(opcode.c_str(), "GET_DEV_POWER")) {
             std::string s_power = doc["power"].GetString();
             m_power = atoi(s_power.c_str());
	}
    }
    if (old != m_status)
        notifyObservers();
    DEBUG_PRINT("::update: " <<m_status);
}
