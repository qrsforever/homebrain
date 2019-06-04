/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "rapidjson.h"
#include "document.h"

#include "KonkeIlluminometer.h"
#include "KonkeInternal.h"
#include "KonkeManager.h"

using namespace std;

static const string MODULE = "KonkeIlluminometer";

void KonkeIlluminometer::dumpInfo()
{
    DEBUG_PRINT("\t" << "nodeid: " << m_nodeid);
    DEBUG_PRINT("\t" << "grade: " << m_grade);
    DEBUG_PRINT("\t" << "illumination: " << m_illumination);
}

void KonkeIlluminometer::get(std::string &info)
{
#if 0
    stringstream ss;
    ss<<grade<< "/" << illumination;
    info=ss.str();
    DEBUG_PRINT("Konkeillumination::get: " <<info);
#endif
}

void KonkeIlluminometer::put(std::string arg)
{
}

void KonkeIlluminometer::update(std::string arg, std::string opcode)
{
    bool notify=false;
    int old_grade = m_grade;
    float old_illumination = m_illumination;

    if(!strcmp(opcode.c_str(), "ILLUMINOMETER_NUMERICAL")) {
        rapidjson::Document doc;
        doc.Parse<0>(arg.c_str());
        std::string s_grade = doc["grade"].GetString();
        std::string s_illumination = doc["illumination"].GetString();
        m_grade = atoi(s_grade.c_str());
        m_illumination = atof(s_illumination.c_str());
    }else if(!strcmp(opcode.c_str(), "ILLUMINOMETER_GRADE")) {
        m_grade = atoi(arg.c_str());
    }else
        return;
    if (old_grade != m_grade)
	    notify = true;
    if (old_illumination != m_illumination)
	    notify = true;
    if(notify)
        notifyObservers();
    DEBUG_PRINT("Konkeillumination::update: grade= " <<m_grade <<"; illumination= "<<m_illumination);
}
