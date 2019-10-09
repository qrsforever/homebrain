/*
 * LarfeInternal.h
 *
 */

#ifndef LARFE_INTERNAL_H_
#define LARFE_INTERNAL_H_

#include <stdint.h>
#include <iostream>
#include <string>
#include <map>
#include <functional>

#define OBJ_CONTROL_BYTE    0x7e
#define OBJ_STATUS_BYTE     0x23
#define OBJ_END_BYTE        0x0d

#define OBJ_ADDR0_SHIFT     3
#define OBJ_ADDR1_MASK      0x7

#define COMMAND_TYPE_MASK  (0xff << COMMAND_TYPE_SHIFT)
#define OBJ_TYPE_MASK      (0xff << OBJ_TYPE_SHIFT)
#define OBJ_ADDR_MASK      (0xffff << OBJ_ADDR_MASK)
#define OBJ_ADDR0_MASK      (0x78 << OBJ_ADDR0_SHIFT)
#define OBJ_ADDR2_MASK      (0xff << OBJ_ADDR2_SHIFT)
#define OBJ_VALUE_MASK     (0xffff << OBJ_VALUE_SHIFT)
#define OBJ_CHECKSUM_MASK  (0xff << OBJ_CHECKSUM_SHIFT)

typedef enum {
    COMMAND_STATUS   = 0x23,
    COMMAND_QUERY_OR_CONTROL = 0x7e,
} CommandType;

typedef enum {
    OBJ_QUERY   =       0x0,
    OBJ_SWITCH  =       0x1,
    OBJ_TUNE_BRI=       0x2,
    OBJ_AIRCONDITION=   0x3,
    OBJ_TEMP_LUX=       0x5,
    OBJ_SCENE     =     0x6,
    OBJ_CURTAIN =       0x7,
    OBJ_DOORCONTACT =   0x9,
    OBJ_SENSOR  =       0xa,
    OBJ_MAX     =       0xff
} ObjectType;

typedef struct {
    std::string m_id;
    ObjectType m_type;
    unsigned char m_eis;
    std::string m_roomId;
    std::string m_name;
    std::string m_value;
    std::string m_addrSet;
    std::string m_addrStatus;
    int m_addrValueSet[3];
    int m_addrValueStatus[3];
} LarfeObj;

typedef struct {
    std::string m_id;
    std::string m_name;
} LarfeRoom;

#endif /* LARFE_INTERNAL_H_ */
