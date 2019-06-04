/*
 * KonkeManager.cpp
 *
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>

#include "rapidjson.h"
#include "document.h"
#include "reader.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"

#include "EH_ServerSdk.h"
#include "EH_Client.h"
#include "KonkeDevice.h"
#include "KonkeManager.h"
#include "KonkeInternal.h"
#include "KonkeDeviceFactory.h"

static const char *NO_SPECIFIC = "*";
static const char *STATUS = "status";

using namespace std;
using namespace rapidjson;

static const string MODULE = "KKManager";
KonkeManager *KonkeManager::m_instance = NULL;

KonkeManager::KonkeManager()
{
    m_kkCcu = new KonkeCcu();
}

KonkeManager::~KonkeManager()
{
    delete m_kkCcu;
}

KonkeManager *KonkeManager::getInstance()
{
    if (m_instance == NULL)
        m_instance = new KonkeManager();

    return m_instance;
}

void KonkeManager::clientInit(EH_Client * client, string id, string accessKey)
{
    client->init(id.c_str(), accessKey.c_str(), m_instance, true);
    m_client = client;
}
void KonkeManager::getDevices(std::map < std::string, KonkeDevice::Ptr > &devices)
{
    devices = m_devices;
}

KonkeDevice::Ptr KonkeManager::findDevice(std::string nodeid)
{
    KonkeDevice::Ptr pDevice;
    std::map < std::string, KonkeDevice::Ptr >::iterator it;
    it = m_devices.find(nodeid);
    if (it != m_devices.end()) {
	    DEBUG_PRINT("node " << nodeid.c_str() <<" found");
	    return it->second;
    }else
	    return NULL;
}

void KonkeManager::onClientStatusChanged(EH_Client::ClientStatus currentStatus)
{
    /* TODO: Using status to control */
    std::string work("WORKING");
    std::string clientstatus = EH_Client::getClientStatusStr(currentStatus);
    WARNING_PRINT("Status Changed :" << clientstatus);
    if(!clientstatus.compare(work)) {
	DEBUG_PRINT("WORKING");
	syncInfo();
	getCcuInfo();
    }
}

void KonkeManager::parseCcuInfo(string json)
{
    DEBUG_PRINT("parseCcuInfo");
    rapidjson::Document doc;
    doc.Parse<0>(json.c_str());
    DEBUG_PRINT("json: " << json);
    m_kkCcu->m_ip = doc["local_ip"].GetString();
    m_kkCcu->m_cloudStatus = doc["cloud_status"].GetString();
    m_kkCcu->m_wanStatus = doc["wan_status"].GetString();
    DEBUG_PRINT("ip: " << m_kkCcu->m_ip);

    if (doc.HasMember("gw_list")) {
        rapidjson::Value& list = doc["gw_list"];
        if (list.IsArray() && !list.Empty()) {
            for (rapidjson::SizeType i = 0; i < list.Size(); i++) {
                rapidjson::Value& gateway = list[i];
                if (gateway.IsObject()) {
                    KonkeGateway::Ptr pGateway;
                    pGateway = std::make_shared < KonkeGateway > ();
                    pGateway->m_name = gateway["gw_name"].GetString();
                    pGateway->m_nodeid = gateway["gw_nodeid"].GetString();
                    pGateway->m_ip = gateway["gw_ip"].GetString();
                    pGateway->m_linked = gateway["gw_link"].GetString();
                    pGateway->m_mac = gateway["gw_mac"].GetString();
                    pGateway->m_type = gateway["gw_type"].GetString();
	            m_kkCcu->addGateway(pGateway);
                }
            }
        }
    }

    m_kkCcu->dumpInfo();
}
void KonkeManager::parseNewDevices(string json)
{
    DEBUG_PRINT("parseNewDevices start");
    rapidjson::Document doc;
    doc.Parse<0>(json.c_str());
    if (doc.HasMember("new_devices")) {
        rapidjson::Value &devices = doc["new_devices"];
        if (devices.IsArray() && !devices.Empty()) {
            for (rapidjson::SizeType i = 0; i < devices.Size(); i++) {
                rapidjson::Value& device = devices[i];
                if (device.IsObject()) {
                    if (device.HasMember("nodeid") && device.HasMember("operate_type") && device.HasMember("status")) {
			DEBUG_PRINT("device nodeid is" << device["nodeid"].GetString());
			std::string deviceid(device["nodeid"].GetString());
                        KonkeDevice::Ptr pDevice;
                        //if (!strcmp(device["operate_type"].GetString(), SWITCHID)) {
                            pDevice = findDevice(deviceid);
			    if (pDevice == NULL) {
				    pDevice = KonkeDeviceFactory::getInstance(device["operate_type"].GetString());
			            if (pDevice == NULL) {
					   continue;
				    }
	                            pDevice->m_nodeid = deviceid;
	                            m_devices[pDevice->m_nodeid] = pDevice;
			    }
			    //pDevice->m_nodeid = deviceid;
			    pDevice->m_channel = device["channel"].GetString();
			    pDevice->m_mac = device["mac"].GetString();
			    pDevice->m_gwmac = device["gw_mac"].GetString();
			    pDevice->m_optype = device["operate_type"].GetString();
			    pDevice->m_uri = KONKE_PREFIX + pDevice->m_mac +"/"+ pDevice->m_channel;
			    pDevice->setManager(this);

                            DEBUG_PRINT("device operate_type is "<< device["operate_type"].GetString());
                            rapidjson::Value &status = device["status"];
                            if (status.HasMember("arg")) {
                                rapidjson::Value &status_arg = status["arg"];
			        StringBuffer buffer;
			        Writer<StringBuffer> writer(buffer);
			        status_arg.Accept(writer);
                                DEBUG_PRINT("buffer="<< buffer.GetString());
			        //pDevice->update(status["arg"].GetString());
			        pDevice->update(buffer.GetString(),status["opcode"].GetString());
                            }
                            pDevice->dumpInfo();
                    }
                }
            }
        }
    }

    DEBUG_PRINT("parseNewDevices end");
    return;
}

void KonkeManager::parseSyncInfo(string json)
{
    DEBUG_PRINT("parseSyncInfo");
#if 0
    string deviceInfo = json["devices"];
    if (deviceInfo.empty() == false)
        parseDeviceInfo(deviceInfo);
    string deviceStatus = json["device_status"];
    if (deviceStatus.empty() == false)
        parseDeviceStatus(deviceStatus);
#endif
}

void KonkeManager::parseDeviceInfo(string json)
{
    DEBUG_PRINT("parseDeviceInfo");
    return;
}

void KonkeManager::parseDeviceStatus(string json)
{
}

void KonkeManager::onRecvMsg(string nodeId, string opcode, string arg,
                             string status)
{
    DEBUG_PRINT("recv msg, nodeId:" << nodeId << ", opcode:" << opcode <<
                ", status:" << status);
    DEBUG_PRINT("arg: " << arg);

    if (opcode == "GET_CCU_INFO") {
        if (status == "success")
            parseCcuInfo(arg);
    } else if (opcode == "SYNC_INFO") {
        if (status == "success")
            parseSyncInfo(arg);
    } else if (opcode == "NEW_DEVICES") {
        if (status == "success")
		parseNewDevices(arg);
    } else if (opcode == "OPEN_NET_C HANNEL") {
            DEBUG_PRINT("OPEN_NET_C HANNEL, nodeid: " << nodeId);
    }else {
	//update node status
        if (status == "success") {
                KonkeDevice::Ptr pDevice;
                pDevice = findDevice(nodeId);
		if (pDevice != NULL) {
		   pDevice->update(arg, opcode);
		}
	}
    }

}

void KonkeManager::sendMessage(const char* arg1, const char* arg2, const char* arg3, const char*arg4)
{
	if (m_client) {
		(static_cast<EH_Client *>(m_client))->sendRequest(arg1, arg2, arg3, arg4);
	} else {
	       	DEBUG_PRINT("Error: SendMessage requested before initializing");
	}
}
void KonkeManager::syncInfo()
{
    sendMessage("*", "SYNC_INFO", "*", "HJ_Config");
}

void KonkeManager::getCcuInfo()
{
    sendMessage("*", "GET_CCU_INFO","*", "HJ_Server");
}

void KonkeManager::openNetChannel()
{
    for (auto netnode : m_kkCcu->m_gateways) {
        KonkeGateway::Ptr pGateway = netnode.second;
        sendMessage(pGateway->m_nodeid.c_str(), "OPEN_NET_CHANNEL", "*", "HJ_Server");
	DEBUG_PRINT("open net channel: "<< pGateway->m_nodeid.c_str());
    }
}

