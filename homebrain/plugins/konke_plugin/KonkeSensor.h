/*
 * KonkeSensor.h
 *
 */

#ifndef KONKE_SENSOR_H_
#define KONKE_SENSOR_H_

#include "KonkeDevice.h"

class KonkeSensor:public KonkeDevice {
  public:
    float m_value;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
