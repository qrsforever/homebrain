/***************************************************************************
 *  LarfeClientLog.h -
 *
 *  Created: 2019-05-20 17:27:12
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LarfeClientLog_H__
#define __LarfeClientLog_H__

#include "Log.h"

#ifdef __cplusplus

extern int gLarfeClientModuleLevel;

#define LC_LOGA(args...)  _LOGA(gLarfeClientModuleLevel, args)
#define LC_LOGE(args...)  _LOGE(gLarfeClientModuleLevel, args)
#define LC_LOGW(args...)  _LOGW(gLarfeClientModuleLevel, args)
#define LC_LOGD(args...)  _LOGD(gLarfeClientModuleLevel, args)
#define LC_LOGI(args...)  _LOGI(gLarfeClientModuleLevel, args)
#define LC_LOGT(args...)  _LOGT(gLarfeClientModuleLevel, args)
#define LC_LOGTT()        _LOGT(gLarfeClientModuleLevel, "run here!\n")

#endif /* __cplusplus */

#endif /* __LarfeClientLog_H__ */
