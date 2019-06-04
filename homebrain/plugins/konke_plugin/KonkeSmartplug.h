/*
 * KonkeSmartplug.h
 */

#ifndef KONKE_SMARTPLUG_H_
#define KONKE_SMARTPLUG_H_

#include "KonkeDevice.h"

class KonkeSmartplug:public KonkeDevice {
  public:
    bool m_status;
    int m_electrical;
    int m_voltage;
    int m_power;
    bool m_powerOnResume;
    bool m_overloadAutoOff;
    float m_overloadPowerThreshold;
    int m_loadDetectionPowerThreshold;

    virtual void get(std::string &info);
    virtual void put(std::string arg);
    virtual void update(std::string arg, std::string opcode);
    virtual void dumpInfo();
};

#endif
