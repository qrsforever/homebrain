/***************************************************************************
 *  HBCloudManager.h - HBCloud Manager Header
 *
 *  Created: 2018-08-06 10:33:34
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __HBCloudManager_H__
#define __HBCloudManager_H__

#include <string>
#include "HBDeviceManager.h"
#ifdef __cplusplus

using namespace OIC::Service::HB;

namespace HB {

class HBCloudManager {
public:
    HBCloudManager();
    ~HBCloudManager();

    virtual int init() { return 0; }

    virtual bool propertyControl(const std::string &deviceId, const std::string &propertyKey, const std::string &value);
    virtual bool propertyReport(const std::string &deviceId, const std::string &deviceType, const std::string &propertyKey, const std::string &value);
    virtual bool statusReport(const std::string &deviceId, const std::string &deviceType, const HBDeviceStatus &status);

private:

}; /* class HBCloudManager */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __HBCloudManager_H__ */
