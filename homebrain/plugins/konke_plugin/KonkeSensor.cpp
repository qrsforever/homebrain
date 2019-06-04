/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeSensor.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeSensor";

void KonkeSensor::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "value: " << m_value);
}

void KonkeSensor::get(std::string &info)
{
    stringstream ss;
    ss<<m_value;
    info=ss.str();
    DEBUG_PRINT("::get: " <<info);
}

void KonkeSensor::put(std::string arg)
{
}

void KonkeSensor::update(std::string arg, std::string opcode)
{
    float old = m_value;
    float m_value = atof(arg.c_str());

    if (old != m_value)
        notifyObservers();
    DEBUG_PRINT("::update: " <<m_value);
}
