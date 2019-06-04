/*
 * KonkeDoorContact.h
 *
 */

#ifndef KONKE_DOORCONTACT_H_
#define KONKE_DOORCONTACT_H_

#include "KonkeDevice.h"

class KonkeDoorContact:public KonkeDevice {
  public:
    bool m_status;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
