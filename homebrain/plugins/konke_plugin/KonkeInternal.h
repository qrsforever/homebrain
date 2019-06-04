/*
 * KonkeInternal.h
 *
 */

#ifndef KONKE_INTERNAL_H_
#define KONKE_INTERNAL_H_

#include <iostream>

//#define NDEBUG

#define WARNING_PRINT(x) do { std::cout << MODULE << ": " << x << std::endl; } while (0)
#define ERROR_PRINT(x) do { std::cout << "ERROR " << MODULE << ": " << x << std::endl; } while (0)

#ifdef NDEBUG
#define DEBUG_PRINT(x) do { std::cout << MODULE << ": " << x << std::endl; } while (0)
#else
#define DEBUG_PRINT(x)
#endif

static std::map < std::string, std::string > KonkeIconNameMap = {
    {"temp_s_icon", "temperature"},
    {"humi_s_icon", "humidity"},
    {"illum_s_icon", "illumination"},
    {"t_3", "doorcontact"},
    {"sos", "alarm"},
};

#endif /* KONKE_INTERNAL_H_ */
