/***************************************************************************
 *  NotifyPayload.h -
 *
 *  Created: 2019-07-02 20:53:47
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __NotifyPayload_H__
#define __NotifyPayload_H__

#include "Payload.h"

#ifdef __cplusplus

namespace HB {

class NotifyPayload : public Payload {
public:
    NotifyPayload();
    ~NotifyPayload();
    PayloadType type() { return PT_NOTIFY_PAYLOAD; }

    std::string mID;
    std::string mTitle;
    std::string mContent;

}; /* class NotifyPayload */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __NotifyPayload_H__ */
