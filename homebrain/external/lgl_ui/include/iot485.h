#ifndef _IOT485_H_
#define _IOT485_H_
#include <map>
#include <string>
#include "iotSock.h"
using namespace std;

#define RS485_INTERFACE_TYPE     RS485
#define RS485_BAUD_RATE          9600
#define RS485_CHECK_BIT          0
#define RS485_STOP_BIT           1

typedef enum
{
    //对LIGHT 控制
    E_CON_LIGHT_ON = 0x00,
    E_CON_LIGHT_OFF = 0x01,
}E_CON_LIGHT;

typedef enum
{
    //对CURTAIN 控制
    E_CON_CURTAIN_ON = 0x00,
    E_CON_CURTAIN_OFF = 0x01,
    E_CON_CURTAIN_STOP = 0x02,
}E_CON_CURTAIN;

typedef enum
{
    //对SCENE 控制
    E_CON_SCENE_LEARN = 0x00,
    E_CON_SCENE_CALL = 0x01,
    E_CON_SCENE_EXIT = 0x02,
}E_CON_SCENE;

typedef enum
{
    //AIR 控制
    E_CON_AIR_ON = 0x01,
    E_CON_AIR_OFF = 0x00,
}E_CON_AIR;

typedef enum
{
    //FLOOR控制
    E_CON_FLOOR_ON = 0x01,
    E_CON_FLOOR_OFF = 0x00,
}E_CON_FLOOR;

typedef enum
{
    E_CON_DEV_SETDODING = 0x01,
    E_CON_DEV_SETDONE = 0x00,
}E_CON_DEV;

typedef enum
{
    //对从设备管理控制
    E_CON_SLAVE_SEARCH = 0x10,
    E_CON_SLAVE_ACK = 0x11,
    E_CON_PERMIT_SLAVE = 0x12,
    E_CON_SLAVE_ACKDONE = 0x13,

    //灯控制
    E_CON_LIGHT_SET = 0x20,
    E_CON_LIGHT_UPDATE = 0x30,
    E_CON_LIGHT_STATUS = 0x40,

    //窗帘控制
    E_CON_CURTAIN_SET = 0x21,
    E_CON_CURTAIN_UPDATE = 0x31,
    E_CON_CURTAIN_STATUS = 0x41,

    //空调控制
    E_CON_AIR_SET = 0x22,
    E_CON_AIR_UPDATE = 0x32,
    E_CON_AIR_STATUS = 0x42,

    //场景控制
    E_CON_SCENE_SET = 0x23,
    E_CON_SCENE_STATUS = 0x33,

    //新风系统控制
    E_CON_NEWAIR_SET = 0x24,
    E_CON_NEWAIR_UPDATE = 0x34,
    E_CON_NEWAIR_STATUS = 0x44,

    //地暖控制
    E_CON_FLOOR_SET = 0x25,
    E_CON_FLOOR_UPDATE = 0x35,
    E_CON_FLOOR_STATUS = 0x45,

    //风机盘管
    E_CON_AIRFAN_SET = 0x2D,
    E_CON_AIRFAN_UPDATE = 0x3D,

    //温度电磁阀
    E_CON_TEMPER_ELEC_SET = 0x2E,
    E_CON_TEMPER_ELEC_UPDATE = 0x3E,

    //温度电动阀
    E_CON_TEMPER_EMOTOR_SET = 0x2F,
    E_CON_TEMPER_EMOTOR_UPDATE = 0x3F,

    E_CONTROL_STX = 0xAA,
}EN_CONTROL_CMD;

//New air types 分类
typedef enum
{
    E_NEWAIR_ONOFF = 0x00,
    E_NEWAIR_SETPM25 = 0x01,
    E_NEWAIR_SETCO2 = 0x02,
    E_NEWAIR_GETPM25 = 0x03,
    E_NEWAIR_GETCO2 = 0x04,
    E_NEWAIR_GETHCHO= 0x05,
    E_NEWAIR_SETHCHO= 0x0A,
    E_NEWAIR_SETMODWIND = 0x06,
    E_NEWAIR_SETHUMIDITYLOW = 0x07,
    E_NEWAIR_SETHUMIDITYHIGH = 0x08,
    E_NEWAIR_GETHUMIDITY = 0x09,
}EN_NEWAIR_TYPE;

//device ID 分类
typedef enum
{
    E_DEV_LIGHT = 0x0000,
    E_DEV_CURTAIN = 0x0100,
    E_DEV_SCENE = 0x0200,
    E_DEV_AIR = 0x0300,
    E_DEV_NEWAIR = 0x0400,
    E_DEV_FLOOR = 0x0500,
    E_DEV_AIRFAN = 0x0600,
    E_DEV_TEMPER_ELEC = 0x0700,
    E_DEV_TEMPER_EMOTOR = 0x0800,
}EN_DEV_ID;

//class Interface_Control;

//class Iot_Sock;
class Iot_485
{
public:
    Iot_485();
    virtual ~Iot_485();
    BOOL Iot_485_setupMapAddr();
    BOOL Iot_485_updateAddr(INT id, U8 *buf);
    void Iot_485_checkSum(char*buf);
    void Iot_485_registerDevAddr();
    void Iot_485_parseCmd(U8 *recvbuf, INT len);
    BOOL Iot_485_masterSlaveComm(U8 *buf);
    BOOL Iot_485_conLight(U8 *buf);
    BOOL Iot_485_conCurtain(U8 *buf);
    BOOL Iot_485_conScene(U8 *buf);
    BOOL Iot_485_conAir(U8 *buf);
    BOOL Iot_485_conNewair(U8 *buf);
    BOOL Iot_485_conFloor(U8 *buf);
    BOOL Iot_485_conTempElec(U8 *buf);
    BOOL Iot_485_conTempMot(U8 *buf);

public :
    EN_CONTROL_CMD cmdControl;  //接受的IOT cmd
    INT buflen;
    U8 checkSum;

    U8 regbuf[32];
    U8 lightbuf[8];
    U8 cutainbuf[8];
    U8 scenebuf[8];
    U8 airbuf[11];
    U8 floorbuf[8];
    U8 newairbuf[10];

    E_CON_DEV flagLight;
    E_CON_DEV flagCurtain;
    E_CON_DEV flagScene;
    E_CON_DEV flagAir;
    E_CON_DEV flagFloor;
    E_CON_DEV flagNewAir;

    static int count_test;

    U8 slaveAddr;
    map<INT, UINT16> mpaddr;
//    Interface_Control *ifaceContorl;
    Iot_Sock *iotSock485;
};

#endif
