/***************************************************************************
 *  NotifyDataChannel.h -
 *
 *  Created: 2019-07-02 20:57:49
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __NotifyDataChannel_H__
#define __NotifyDataChannel_H__

#include "DataChannel.h"

#ifdef __cplusplus

namespace HB {

class NotifyDataChannel : public DataChannel {
public:
    NotifyDataChannel();
    ~NotifyDataChannel();

    int init();

    virtual bool send(int action, std::shared_ptr<Payload> payload);

protected:
}; /* class NotifyDataChannel */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __NotifyDataChannel_H__ */
