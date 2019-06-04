/*
 * KonkeDevice.cpp
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeDevice.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KKDevice";

KonkeDevice::KonkeDevice()
{

}

KonkeDevice::~KonkeDevice()
{

}
void KonkeDevice::sendMessage(const char *cmd, const char *arg)
{
    if(m_manager)
        static_cast<KonkeManager *>(m_manager)->sendMessage(m_nodeid.c_str(), cmd, arg, "HJ_Server");
}
void KonkeDevice::dumpInfo()
{
    if (KonkeIconNameMap[m_icon].empty())
        DEBUG_PRINT("Dump Unkown Device info:");
    else
        DEBUG_PRINT("Dump Device " << KonkeIconNameMap[m_icon] << " info:");

    DEBUG_PRINT("\t" << "icon: " << m_icon);
    DEBUG_PRINT("\t" << "name: " << m_name);
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "mac: " << m_mac);
    DEBUG_PRINT("\t" << "room: " << m_room);
    DEBUG_PRINT("\t" << "gateway: " << m_gateway->m_name);
}
