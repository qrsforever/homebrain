/*
 * KonkeDevice.h
 *
 */

#ifndef KONKE_DEVICE_H_
#define KONKE_DEVICE_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "KonkeCcu.h"
#include "EH_Client.h"

using namespace std;
extern void KonkeNotifyObservers(std::string uri);

#define KONKE_SWITCH_ID  "1"
#define KONKE_SOSPANEL_ID  "3502"
#define KONKE_TEMP_SENSOR_ID  "10001"
#define KONKE_HUMIDITY_SENSOR_ID  "10002"
#define KONKE_ILLUMINATION_ID "10003"
#define KONKE_SCENEPANEL_ID  "3501"
#define KONKE_DOORCONTACT_ID  "3499"
#define KONKE_CURTAIN_ID "1001"
#define KONKE_WISTAR_ID "1005"
#define KONKE_SMARTPLUG_ID "2003"

class KonkeManager;
class KonkeDevice {
  public:
    typedef std::shared_ptr < KonkeDevice > Ptr;
    std::string m_icon;
    std::string m_name;
    std::string m_nodeid;
    std::string m_mac;
    std::string m_gwmac;
    std::string m_optype;
    std::string m_channel;
    std::string m_room;

    std::string m_uri;
    std::shared_ptr < KonkeGateway > m_gateway;

    void notifyObservers() { KonkeNotifyObservers(m_nodeid);}
    virtual void get(std::string &info) = 0;
    virtual void put(std::string arg) = 0;
    virtual void update(std::string arg, std::string opcode) = 0;

    void sendMessage(const char *cmd, const char *arg);
    void setManager(KonkeManager* manager) { m_manager = manager;}
    void* m_manager;

    KonkeDevice();
    virtual ~ KonkeDevice();
    virtual void dumpInfo();
};

#endif /* KONKE_DEVICE_H_ */
