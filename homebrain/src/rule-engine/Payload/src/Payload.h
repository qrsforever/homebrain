/***************************************************************************
 *  Payload.h -  Payload Header
 *
 *  Created: 2018-06-19 12:38:34
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __Payload_H__
#define __Payload_H__

#include "Object.h"

#include <string>

#ifdef __cplusplus

namespace HB {

typedef enum {
    PT_INSTANCE_PAYLOAD,
    PT_RULE_PAYLOAD,
    PT_CLASS_PAYLOAD,
    PT_TIMER_PAYLOAD,
} PayloadType;

class Payload : public ::UTILS::Object {
public:
    Payload() {}
    virtual ~Payload() {}
    virtual PayloadType type() = 0;
}; /* class Payload */

std::string innerOfInsname(std::string name);
std::string outerOfInsname(std::string name);

std::string innerOfRulename(std::string name);
std::string outerOfRulename(std::string name);

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __Payload_H__ */
