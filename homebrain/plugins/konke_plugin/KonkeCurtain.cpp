/*
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeCurtain.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeCurtain";

void KonkeCurtain::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "status: " << m_status);
}

void KonkeCurtain::get(std::string &info)
{
    if(m_status == CURTAIN_OPEN)
	    info = "OPEN";
    if(m_status == CURTAIN_CLOSE)
	    info = "CLOSE";
    if(m_status == CURTAIN_STOP)
	    info = "STOP";
    DEBUG_PRINT("::get: " <<info);
}
void KonkeCurtain::put(std::string arg)
{
   sendMessage("SWITCH", arg.c_str());
}

void KonkeCurtain::update(std::string arg, std::string opcode)
{
    int old_status = m_status;
    if (!strcmp(arg.c_str(), "OPEN")) {
	    m_status = CURTAIN_OPEN;
    }else if (!strcmp(arg.c_str(), "CLOSE")) {
	    m_status = CURTAIN_CLOSE;
    }else if (!strcmp(arg.c_str(), "STOP")) {
	    m_status = CURTAIN_STOP;
    }else {
	    return;
    }
    if (old_status != m_status)
        notifyObservers();
    DEBUG_PRINT("::update: " <<m_status);
}
