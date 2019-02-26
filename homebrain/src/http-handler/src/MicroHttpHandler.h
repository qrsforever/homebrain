/***************************************************************************
 *  MicroHttpHandler.h - 
 *
 *  Created: 2018-10-24 11:26:54
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __MicroHttpHandler_H__
#define __MicroHttpHandler_H__

#include "Thread.h"
#include "MicroMiddleware.h"

namespace HB {

using APP = crow::App<MicroMiddleware>;

#define REST_GET     crow::HTTPMethod::GET
#define REST_POST    crow::HTTPMethod::POST

class MicroHttpHandler : public UTILS::Thread {
public:
    MicroHttpHandler();
    ~MicroHttpHandler();

    void init(int port, int multi);

    virtual void run();

private:
    int mPort;
    int mMulti;
}; /* class MicroHttpHandler */

int checkToken(const std::string &token);
MicroHttpHandler& httpHandler();

} /* namespace HB */

#endif /* __MicroHttpHandler_H__ */
