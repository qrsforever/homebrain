/*
 * KonkeCcu.h
 *
 */

#ifndef KONKE_CCU_H_
#define KONKE_CCU_H_

#include <string>
#include <vector>
#include <memory>
#include <map>

using namespace std;

class KonkeGateway {
  public:
    typedef std::shared_ptr < KonkeGateway > Ptr;
     std::string m_name;
     std::string m_nodeid;
     std::string m_ip;
     std::string m_linked;
     std::string m_mac;
     std::string m_type;
     std::vector < string > m_deviceList;

     void update(KonkeGateway::Ptr gateway);
     KonkeGateway();
     virtual ~ KonkeGateway();
};

class KonkeCcu {
  public:
    std::string m_ip;
    std::string m_cloudStatus;
    std::string m_wanStatus;
    int m_nGateway;
    typedef std::map < std::string, KonkeGateway::Ptr >::iterator gatewayItr;
     std::map < std::string, KonkeGateway::Ptr > m_gateways;
    void addGateway(KonkeGateway::Ptr pGateway);

     KonkeCcu();
     virtual ~ KonkeCcu();
    void dumpInfo();
};

#endif /* KONKE_CCU_H_ */
