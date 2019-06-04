/***************************************************************************
 *  GatewayAgentHandler.h -
 *
 *  Created: 2019-05-21 18:25:15
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __GatewayAgentHandler_H__
#define __GatewayAgentHandler_H__

#include "MessageHandler.h"

#define UNITTEST 0

#define GA_NETWORK_OK           1
#define GA_CONNECT_AGAIN        2

#define GA_UNITTEST            99

#ifdef __cplusplus

namespace HB {

class GatewayAgentHandler : public ::UTILS::MessageHandler {
protected:
    void handleMessage(::UTILS::Message *msg);
private:

#if UNITTEST
    void doUnitTest(::UTILS::Message *msg);
#endif
}; /* class GatewayAgentHandler */

GatewayAgentHandler& GAHandler();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __GatewayAgentHandler_H__ */
