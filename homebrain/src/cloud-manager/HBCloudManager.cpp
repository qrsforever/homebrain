/***************************************************************************
 *  HBCloudManager.cpp - HBCloud Manager Impl
 *
 *  Created: 2018-08-06 10:37:50
 *
 *  Copyright QRS
 ****************************************************************************/

#include "HBCloudManager.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "StringData.h"
#include "Log.h"

#include <stdlib.h>
#include <memory>

using namespace UTILS;

namespace HB {

HBCloudManager::HBCloudManager()
{
}

HBCloudManager::~HBCloudManager()
{

}

bool HBCloudManager::propertyControl(const std::string &deviceId, const std::string &propertyKey, const std::string &value)
{
    LOGD("(%s %s %s)\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
    std::shared_ptr<StringData> fact;
    if (deviceId == "surper.virtual.device") {
        if (propertyKey == "mode") {
            std::string str;
            str.append("(virtualmode ").append(value).append(")");
            fact = std::make_shared<StringData>(str.c_str());
            ruleHandler().sendMessage(ruleHandler().obtainMessage(RET_ASSERT_FACT, fact)); 
        }
    }
    return true;
}

bool HBCloudManager::propertyReport(const std::string &deviceId, const std::string &deviceType, const std::string &propertyKey, const std::string &value)
{
    return false;
}

bool HBCloudManager::statusReport(const std::string &deviceId, const std::string &deviceType, const HBDeviceStatus &status)
{
    return false;
}

} /* namespace HB */
