/***************************************************************************
 *  GatewayAgentLog.cpp -
 *
 *  Created: 2019-05-20 17:27:03
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"

int gGatewayAgentModuleLevel = LOG_LEVEL_DEBUG;

namespace HB {

static LogModule GatewayAgentModule("gateway-agent", gGatewayAgentModuleLevel);

} /* namespace HB */

