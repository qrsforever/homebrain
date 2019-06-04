/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeDoorContact.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeDoorContact";

void KonkeDoorContact::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "status: " << m_status);
}

void KonkeDoorContact::get(std::string &info)
{
    info = m_status?"OPEN":"CLOSE";
    DEBUG_PRINT("::get: " <<info);
}
void KonkeDoorContact::put(std::string arg)
{
}

void KonkeDoorContact::update(std::string arg, std::string opcode)
{
    bool old_status = m_status ;
    rapidjson::Document doc;
    doc.Parse<0>(arg.c_str());
    std::string newstatus = doc["status"].GetString();
    if (!strcmp(newstatus.c_str(), "OPEN")) {
	    m_status = true;
    }else if (!strcmp(newstatus.c_str(), "CLOSE")) {
	    m_status = false;
    }else {
	    return;
    }
    if (old_status != m_status)
        notifyObservers();
    DEBUG_PRINT("::update: " <<m_status);
}
