/*
 * KonkeManager.h
 *
 */

#ifndef KONKE_MANAGER_H_
#define KONKE_MANAGER_H_

#include "EH_Client.h"
#include "KonkeCcu.h"
//#include "KonkeDevice.h"

using namespace std;
#define KONKE_PREFIX "Konke/"
class KonkeManager:public EH_Client::EH_ClientCallBackHandler {
  public:
    static KonkeManager *getInstance();
    void clientInit(EH_Client * client, string id, string accessKey);
    void onClientStatusChanged(EH_Client::ClientStatus currentStatus);
    void onRecvMsg(string nodeId, string opcode, string arg,
                   string status);
    void sendMessage(const char* arg1, const char* arg2, const char* arg3, const char*arg4);
    void syncInfo();
    void getCcuInfo();
    void openNetChannel();

    void getDevices(std::map < std::string, KonkeDevice::Ptr > &devices);
    KonkeDevice::Ptr findDevice(std::string nodeid);

    KonkeManager();
    virtual ~ KonkeManager();
  private:

    void parseCcuInfo(string json);
    void parseSyncInfo(string json);
    void parseDeviceInfo(string json);
    void parseDeviceStatus(string json);
    void parseNewDevices(string json);

    static KonkeManager *m_instance;
    EH_Client *m_client;
    KonkeCcu *m_kkCcu;
    std::map < std::string, KonkeDevice::Ptr > m_devices;
};

#endif /* KONKE_MANAGER_H_ */
