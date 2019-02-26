/***************************************************************************
 *  MicroMiddleware.cpp -
 *
 *  Created: 2018-10-24 13:46:00
 *
 *  Copyright QRS
 ****************************************************************************/

#include "MicroMiddleware.h"
#include "MicroLogHandler.h"

namespace HB {

MicroMiddleware::MicroMiddleware()
{
}

MicroMiddleware::~MicroMiddleware()
{
}

void MicroMiddleware::before_handle(crow::request& req, crow::response& /*res*/, context& /*ctx*/)
{
    // std::string token = req.get_header_value("token");
    /* check token */
    // if (!token.empty())
        // HH_LOGD("token: %s\n", token.c_str());
    /* 3: token invlid */
    /* 4: no token */
}

void MicroMiddleware::after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/)
{
}

} /* namespace HB */
