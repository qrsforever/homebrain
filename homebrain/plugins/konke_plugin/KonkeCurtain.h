/*
 * KonkeCurtain.h
 *
 */

#ifndef KONKE_CURTAIN_H_
#define KONKE_CURTAIN_H_

#include "KonkeDevice.h"
#define CURTAIN_OPEN 1
#define CURTAIN_CLOSE 2
#define CURTAIN_STOP 3
class KonkeCurtain:public KonkeDevice {
  public:
    int m_status;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
