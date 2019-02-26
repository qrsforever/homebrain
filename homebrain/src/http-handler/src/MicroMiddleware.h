/***************************************************************************
 *  MicroMiddleware.h -
 *
 *  Created: 2018-10-24 13:43:13
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __MicroMiddleware_H__
#define __MicroMiddleware_H__

#include "crow.h"

namespace HB {

class MicroMiddleware {
public:
    MicroMiddleware();
    ~MicroMiddleware();

    struct context {
    }; /* struct context */

    void before_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/);
    void after_handle(crow::request& /*req*/, crow::response& /*res*/, context& /*ctx*/);

private:
}; /* class MicroMiddleware */

} /* namespace HB */

#endif /* __MicroMiddleware_H__ */
