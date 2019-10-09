/***************************************************************************
 *  MainHandler.h -
 *
 *  Created: 2019-06-18 13:44:05
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __MainHandler_H__
#define __MainHandler_H__

#include "MessageHandler.h"

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class MainHandler : public MessageHandler {
public:
    MainHandler();
    MainHandler(MessageQueue *queue);
    ~MainHandler();

protected:
    void handleMessage(Message *msg);

private:
    void onLogEvent(Message *msg);
    void onSystemEvent(Message *msg);
    void onDatabaseEvent(Message *msg);

}; /* class MainHandler */

MainHandler& mainHandler();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __MainHandler_H__ */
