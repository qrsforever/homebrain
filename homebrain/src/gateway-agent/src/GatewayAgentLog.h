/***************************************************************************
 *  GatewayAgentLog.h -
 *
 *  Created: 2019-05-20 17:27:12
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __GatewayAgentLog_H__
#define __GatewayAgentLog_H__

#ifndef GA_EXPORT_SDK
#include "Log.h"

#ifdef __cplusplus

extern int gGatewayAgentModuleLevel;

#define GA_LOGE(args...)  _LOGE(gGatewayAgentModuleLevel, args)
#define GA_LOGW(args...)  _LOGW(gGatewayAgentModuleLevel, args)
#define GA_LOGD(args...)  _LOGD(gGatewayAgentModuleLevel, args)
#define GA_LOGI(args...)  _LOGI(gGatewayAgentModuleLevel, args)
#define GA_LOGT(args...)  _LOGT(gGatewayAgentModuleLevel, args)
#define GA_LOGTT()        _LOGT(gGatewayAgentModuleLevel, "run here!\n")

#endif /* __cplusplus */

#else

#include <stdio.h>

#define GA_LOGE(args...)  printf(args)
#define GA_LOGW(args...)  printf(args)
#define GA_LOGD(args...)  printf(args)
#define GA_LOGI(args...)  printf(args)
#define GA_LOGT(args...)  printf(args)
#define GA_LOGTT()        printf("run here!\n")
#endif

#endif /* __HBGatewayAgentLog_H__ */
