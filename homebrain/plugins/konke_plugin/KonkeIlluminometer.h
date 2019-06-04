/*
 * KonkeIlluminometer.h
 *
 */

#ifndef KONKE_ILLUMINOMETER_H_
#define KONKE_ILLUMINOMETER_H_

#include "KonkeDevice.h"

class KonkeIlluminometer:public KonkeDevice {
  public:
    float m_illumination;
    int m_grade;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
