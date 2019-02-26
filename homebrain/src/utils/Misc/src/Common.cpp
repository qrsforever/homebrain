/***************************************************************************
 *  Common.cpp - Common api
 *
 *  Created: 2018-07-13 21:33:03
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Common.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

namespace UTILS {

std::string& stringTrim(std::string& text)
{
    if (!text.empty()) {
        text.erase(0, text.find_first_not_of((" \n\r\t\f\v")));
        text.erase(text.find_last_not_of((" \n\r\t\f\v")) + 1);
    }
    return text;
}

std::string int2String(int num)
{
    char id[32] = { 0 };
    sprintf(id, "%d", num);
    return std::move(std::string(id));
}

std::string double2String(double num)
{
    char id[64] = { 0 };
    sprintf(id, "%f", num);
    return std::move(std::string(id));
}

std::vector<std::string> stringSplit(const std::string &text, const std::string &delim)
{
    std::vector<std::string> ss;
    char *data = (char*)calloc(text.length()+1 , 1);
    strcpy(data, text.c_str());
    if (data) {
        char *ptr = strtok(data, delim.c_str());
        while (ptr) {
            ss.push_back(ptr);
            ptr = strtok(NULL, delim.c_str());
        }
        free(data);
    }
    return std::move(ss);
}

std::string assignSafe(const char *str)
{
    if (!str)
        return std::move(std::string());
    return std::move(std::string(str));
}

std::string genRandom(unsigned int a, unsigned int b)
{
    srand((unsigned)time(0));
    char id[32] = { 0 };
    sprintf(id, "%u", (rand()%(b-a+1))+a);
    return std::move(std::string(id));
}

} /* namespace UTILS */
