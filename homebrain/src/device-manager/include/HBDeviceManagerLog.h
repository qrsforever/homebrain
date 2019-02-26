/***************************************************************************
 *  RuleEngineLog.h - Rule Engine Log
 *
 *  Created: 2018-07-18 15:36:53
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __HBDeviceManagerLog_H__
#define __HBDeviceManagerLog_H__

#include "Log.h"

#ifdef __cplusplus

extern int gDeviceManagerModuleLevel;

#define DM_LOGE(args...)  _LOGE(gDeviceManagerModuleLevel, args)
#define DM_LOGW(args...)  _LOGW(gDeviceManagerModuleLevel, args)
#define DM_LOGD(args...)  _LOGD(gDeviceManagerModuleLevel, args)
#define DM_LOGI(args...)  _LOGI(gDeviceManagerModuleLevel, args)
#define DM_LOGT(args...)  _LOGT(gDeviceManagerModuleLevel, args)
#define DM_LOGTT()        _LOGT(gDeviceManagerModuleLevel, "run here!\n")

#endif /* __cplusplus */

#endif /* __HBDeviceManagerLog_H__ */
