/***************************************************************************
 *  KKTypes.h -
 *
 *  Created: 2019-09-27
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __KKTypes_H__
#define __KKTypes_H__

#include "MessageTypes.h"

#ifdef __cplusplus

enum KKMessageTypes {
    KKMT_HB_REGISTER = MT_EXTERNAL + 1,
    KKMT_HB_GAGENT,
    KKMT_HB_UPGRADE,
    KKMT_HB_UNITTEST,
};

/*-----------------------------------------------------------------
 *   KKMT_HB_REGISTER 
 *-----------------------------------------------------------------*/
#define HB_REGISTER_EVENT_REQUEST   0
#define HB_REGISTER_EVENT_FAIL      1
#define HB_REGISTER_EVENT_SUCCESS   2

/*-----------------------------------------------------------------
 *   KKMT_HB_GAGENT
 *-----------------------------------------------------------------*/
#define HB_GAGENT_CONNECT         0
#define HB_GAGENT_REGISTER_SUBDEV 1
#define HB_GAGENT_SUBDEV_TOPOADD  2
#define HB_GAGENT_SUBDEV_TOPODEL  3
#define HB_GAGENT_UPGRADE_REPORT  4
#define HB_GAGENT_UPGRADE_CHECK   5

/*-----------------------------------------------------------------
 *   KKMT_HB_UPGRADE
 *-----------------------------------------------------------------*/
#define HB_UPGRADE_STATUS         0
#define HB_UPGRADE_HOSTINFOFILE   1
#define HB_UPGRADE_BURNING        2

#if defined(KKGA_UNITTEST)

#define UNITTEST_START           1
#define UNITTEST_REGISTER_SUBDEV 2
#define UNITTEST_SUBDEV_TOPOADD  3
#define UNITTEST_SUBDEV_TOPODEL  4
#define UNITTEST_UPGRADE_REPORT  5
#define UNITTEST_END             10

#endif


/*-----------------------------------------------------------------
 *   KK other macro define
 *-----------------------------------------------------------------*/

#ifdef HAVE_BUILD_INFO
#include "build_info.h"
#define KK_BUILD_VERSION    HB_BUILD_VERSION
#define KK_BUILD_DATETIME   HB_BUILD_DATETIME
#else
#define KK_BUILD_VERSION    "0.0.1"
#define KK_BUILD_DATETIME   "unkown"
#endif

#ifdef EMQ_TEST
#define ELINK_EMQ_HOST       "10.112.36.194"
#define ELINK_CLIENTID       "el.62be978faf876991c152dba094231cc0"
#define ELINK_DEVICE_SECRET  "ds.d638608eb4cd835d3fd5"
#define ELINK_PRODUCT_KEY    "pk.33ef1b64b64"
#else
#define ELINK_EMQ_HOST       "emq-elink.cp21.ott.cibntv.net"
#endif
#define ELINK_CLOUD_HOST     "https://elinkplantform-pub.scloud.le.com/device/register"

#define UPGRADE_CHECK_INTERVAL  7200000

#endif /* __cplusplus */

#endif /* __KKTypes_H__ */
