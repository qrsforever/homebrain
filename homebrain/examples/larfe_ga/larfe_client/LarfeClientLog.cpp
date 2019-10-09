/***************************************************************************
 *  LarfeClientLog.cpp -
 *
 *  Created: 2019-05-20 17:27:03
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"

int gLarfeClientModuleLevel = LOG_LEVEL_DEBUG;

namespace HB {

static LogModule LarfeClientModule("LarfeClient", gLarfeClientModuleLevel);

} /* namespace HB */

