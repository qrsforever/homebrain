/*
 * KonkeCcu.cpp
 *
 */

#include <stdio.h>
#include <unistd.h>

#include "KonkeCcu.h"
#include "KonkeInternal.h"

using namespace std;

static const string MODULE = "KKCcu";

KonkeGateway::KonkeGateway()
{

}

KonkeGateway::~KonkeGateway()
{

}

KonkeCcu::KonkeCcu()
{

}

KonkeCcu::~KonkeCcu()
{

}

void KonkeGateway::update(KonkeGateway::Ptr pGateway)
{
     m_name = pGateway->m_name;
     m_nodeid = pGateway->m_nodeid;
     m_ip = pGateway->m_ip;
     m_linked = pGateway->m_linked;
     m_ip = pGateway->m_ip;
     m_mac = pGateway->m_mac;
     m_type = pGateway->m_type;
}
void KonkeCcu::addGateway(KonkeGateway::Ptr pGateway)
{
	gatewayItr itr;
	itr = m_gateways.find(pGateway->m_name);
	if (itr != m_gateways.end()) {
                KonkeGateway::Ptr pGateway2 = itr->second;
		pGateway2->update(pGateway);
		return ;
	}else {
		m_gateways[pGateway->m_name] = pGateway;
	}
	return;
}

void KonkeCcu::dumpInfo()
{
    DEBUG_PRINT("Dump CCU info:");
    DEBUG_PRINT("\t" << "ip: " << m_ip);
    DEBUG_PRINT("\t" << "cloud status: " << m_cloudStatus);
    DEBUG_PRINT("\t" << "wan status: " << m_wanStatus);

    map < std::string, KonkeGateway::Ptr >::iterator iter;
    for (iter = m_gateways.begin(); iter != m_gateways.end(); iter++) {
        KonkeGateway::Ptr pGateway = iter->second;

        DEBUG_PRINT("Dump Gateway " << iter->first << " info:");
        DEBUG_PRINT("\t" << "name: " << pGateway->m_name);
        DEBUG_PRINT("\t" << "nodeid: " << pGateway->m_nodeid);
        DEBUG_PRINT("\t" << "ip: " << pGateway->m_ip);
        DEBUG_PRINT("\t" << "linked: " << pGateway->m_linked);
        DEBUG_PRINT("\t" << "mac: " << pGateway->m_mac);
        DEBUG_PRINT("\t" << "type: " << pGateway->m_type);
    }
}
