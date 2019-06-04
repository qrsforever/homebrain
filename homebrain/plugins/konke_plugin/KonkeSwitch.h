/*
 * KonkeSwitch.h
 *
 */

#ifndef KONKE_SWITCH_H_
#define KONKE_SWITCH_H_

#include "KonkeDevice.h"

class KonkeSwitch:public KonkeDevice {
  public:
    bool m_power;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
