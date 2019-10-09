/***************************************************************************
 *  Object.h - Object Base Class Header
 *
 *  Created: 2018-05-31 14:23:14
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __Object_H__
#define __Object_H__

#include "RefCnt.h"

#ifndef PLATFORM_LARFE
#define USE_SHARED_PTR
#endif

#ifdef __cplusplus

namespace UTILS {

class Object : public RefCnt {
public:
    Object() : mMagic(0) {}
    int mMagic;
}; /* class Object */

} /* namespace UTILS */

#endif /* __cplusplus */

#endif /* _Object_H_ */
