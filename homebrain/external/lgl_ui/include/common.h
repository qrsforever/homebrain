
#ifndef _COMMON_H_
#define _COMMON_H_

#define L_SCS 1  //定义10 英寸的checksum 取的方式不一样

#define IOT_COMM_INFO(fmt, ...)             do { if ((0) != 0) {printf(fmt, ##__VA_ARGS__);}}while (0);
#define IOT_COMM_DEBUG(fmt, ...)            do { if ((0x03) != 0) {printf(fmt, ##__VA_ARGS__);}}while (0);
#define IOT_COMM_ERROR(fmt, ...)            do { if ((0x07) != 0) {printf(fmt, ##__VA_ARGS__);}}while (0);
#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 1024

//配置sock ip 与端口
#define PORT 5000
#define NETADDR "192.168.0.7"

typedef enum {
    DEVICE_TYPE_LIGHT = 0,
    DEVICE_TYPE_ALARM,
    DEVICE_TYPE_DOORCONTACT,
    DEVICE_TYPE_ENVSENSOR,
    DEVICE_TYPE_CURTAIN,
    DEVICE_TYPE_PLUG,
} Iot_DevType;

typedef unsigned char BOOL;
typedef char U8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT;
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;
#endif
