/***************************************************************************
 *  GatewayAgentHandler.cpp -
 *
 *  Created: 2019-05-21 18:25:36
 *
 *  Copyright QRS
 ****************************************************************************/

#include "GatewayAgentHandler.h"
#include "GatewayAgentLog.h"
#include "GatewayAgent.h"
#include "MessageTypes.h"

using namespace UTILS;

namespace HB {

static GatewayAgentHandler *gGAHandler = 0;

void GatewayAgentHandler::handleMessage(Message *msg)
{
    GA_LOGD("msg(%d, %d, %d)\n", msg->what, msg->arg1, msg->arg2);
    switch (msg->what) {
        case GA_CONNECT_MQTT:
        case GA_CONNECT_AGAIN:
        case GA_NETWORK_OK:
            if (GAgent().restart() < 0) {
                sendEmptyMessageDelayed(GA_CONNECT_AGAIN, 3000);
            } else
                sendMessage(obtainMessage(MT_GA, MQTT_CONNECT_SUCCESS, 0));
            break;
        case GA_NETWORK_ERR:
            sendEmptyMessageDelayed(GA_CONNECT_AGAIN, 3000);
            break;
    }
    return;
}

GatewayAgentHandler& GAHandler()
{
    if (0 == gGAHandler)
        gGAHandler = new GatewayAgentHandler();
    return *gGAHandler;
}

} /* namespace HB */
