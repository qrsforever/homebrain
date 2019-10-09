/***************************************************************************
 *  MessageLooper - Message Looper Impl
 *
 *  Created: 2018-06-02 08:46:57
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MessageLooper.h"
#include "MessageHandler.h"
#include "Message.h"

namespace UTILS {

namespace Looper {

static MessageLooper *gMainLooper = 0;

MessageLooper& getMainLooper()
{
    /* TODO the first call in main thread, eg. MainHandler */
    if (!gMainLooper) {
        gMainLooper = new MessageLooper(pthread_self());
        gMainLooper->start();
        /* gMainLooper->run(); */
    }
    return *gMainLooper;
}

} /* namespace Looper */

void MessageLooper::run()
{
    MessageQueue *queue = getMessageQueue();

    while (true) {
        Message *msg = queue->next();
        if (msg != NULL) {
            if (0 == msg->target) {
                return;
            }
            msg->target->dispatchMessage(msg);
            msg->recycle();
        }
    }
}

} /* namespace UITLS */

