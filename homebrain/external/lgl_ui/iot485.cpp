#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#include "common.h"
#include "iotSock.h"
#include "iot485.h"

Iot_485::Iot_485()
{
    IOT_COMM_INFO("Iot_485 construct\n");
    cmdControl = E_CONTROL_STX;
    slaveAddr = 0x01;   //设定为从设备地址1
    flagLight = E_CON_DEV_SETDONE;
    flagCurtain = E_CON_DEV_SETDONE;
    flagScene = E_CON_DEV_SETDONE;
    flagAir = E_CON_DEV_SETDONE;

//    ifaceContorl = new Interface_Control();
//    ifaceCallback = new Interface_Callback();

     //注册callback
//    ifaceCallback->ifaceControl_callback = ifaceContorl;
//    ifaceCallback->iot485_callback  = this;

     // 设定callback
//    ifaceContorl->devManager->setCallback(ifaceCallback);
}

Iot_485::~Iot_485()
{
//    delete ifaceContorl;
//    delete ifaceCallback;
//    ifaceCallback = NULL;
//    ifaceContorl = NULL;
    IOT_COMM_INFO("~Iot_485 deconstruct\n");
}

BOOL Iot_485::Iot_485_setupMapAddr()
{
    IOT_COMM_INFO("Enter Iot_485_setupMapAddr\n");
    mpaddr.insert(pair<INT, UINT16>(E_DEV_LIGHT+1, 0x100));   //ID+ADDR  LIGHT 1
    mpaddr.insert(pair<INT, UINT16>(E_DEV_LIGHT+2, 0x101));   //ID+ADDR  LIGHT 1
    mpaddr.insert(pair<INT, UINT16>(E_DEV_CURTAIN+1, 0x75));

    map<INT, UINT16>::iterator iter;
    for(iter = mpaddr.begin(); iter != mpaddr.end(); iter++){
        IOT_COMM_INFO("iter->first id=%d, iter->second addr=0x%x\n", iter->first, iter->second);
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_updateAddr(INT id, U8 *buf)
{
    IOT_COMM_INFO("Enter Iot_485_sendCmd id = %d\n", id);
    return TRUE;
}

void Iot_485::Iot_485_checkSum(char *buf)
{
    IOT_COMM_INFO("Enter Iot_485_checkSum len = %d\n", buf[2]);

    char *p = buf;
    INT len = p[2]+3;
    UINT sum = 0;
#ifdef L_SCS
    len-=1;
    p++;
#endif
    while(len > 0)
    {
        sum =sum+*p;
        p+= 1;
        len -= 1;
    }

    while (sum>>8)
    sum=(sum&0xff) + (sum>>8);

    //填充checksum
    buf[buf[2]+3] = (0x00ff)&sum;
}

void Iot_485::Iot_485_registerDevAddr()
{
    IOT_COMM_INFO("Enter Iot_485_registerDevAddr\n");

    map<INT, UINT16>::iterator iter;
    for(iter = mpaddr.begin(); iter != mpaddr.end(); iter++){
        memset(regbuf,0x0,32*sizeof(U8));
        IOT_COMM_INFO("iter->first addr= 0x%x, iter->second=0x%x\n", iter->first, iter->second);
        switch((iter->first)&0xFF00)
        {
            regbuf[0] = E_CONTROL_STX;
            case  E_DEV_LIGHT:
                  regbuf[1] = E_CON_LIGHT_UPDATE;
                  regbuf[2] = 0x03;
                  regbuf[3] = (iter->second)&0xF0>>8;
                  regbuf[4] = (iter->second)&0x0F;
                  regbuf[5] = 0x00;
            case  E_DEV_CURTAIN:
                  regbuf[1] = E_CON_CURTAIN_UPDATE;
                  regbuf[2] = 0x03;
                  regbuf[3] = (iter->second)&0xF0>>8;
                  regbuf[4] = (iter->second)&0x0F;
                  regbuf[5] = 0x04;
            case  E_DEV_AIR:
                  regbuf[1] = 0x32;
                  regbuf[2] = (iter->second)&0xF0>>8;
                  regbuf[3] = (iter->second)&0x0F;
            case  E_DEV_NEWAIR:
                  regbuf[1] = 0x34;
                  regbuf[2] = (iter->second)&0x0F;
            case  E_DEV_AIRFAN:
                  regbuf[1] = 0x3D;
            case  E_DEV_TEMPER_ELEC:
                  regbuf[1] = 0x3E;
            case  E_DEV_TEMPER_EMOTOR:
                  regbuf[1] = 0x3F;
        }
        Iot_485_checkSum(regbuf);

        //send
        //Iot_Sock::Iot_Sock_sendbuf(regbuf);
    }
}

void Iot_485::Iot_485_parseCmd(U8 *recvbuf, INT len)
{
    IOT_COMM_INFO("Enter Iot_485_parseCmd\n");
    U8 parsebuf[1024] = {0};
    INT i,j;
    memcpy(parsebuf, recvbuf, len);
    if(len==1)
	{
        return;
    }

    //0xaa 0x20 0x3 0x0 0x60 0x0 0x2d      0xaa 0x10 0x1 0x1 0xbc
    for(i=0, j=0; i<len; i=i+j)
    {
        if(parsebuf[i] == 0xaa)
        {
            switch(parsebuf[i+1])
            {
                case E_CON_SLAVE_SEARCH:
                case E_CON_PERMIT_SLAVE:
                {
                    Iot_485_masterSlaveComm(&parsebuf[i]);
                    break;
                }
                case E_CON_LIGHT_SET:
                case E_CON_LIGHT_STATUS:
                {
                    Iot_485_conLight(&parsebuf[i]);
                    break;
                }

                case E_CON_CURTAIN_SET:
                case E_CON_CURTAIN_STATUS:
                {
                    Iot_485_conCurtain(&parsebuf[i]);
                    break;
                }

                case E_CON_SCENE_SET:
                case E_CON_SCENE_STATUS:
                {
                    Iot_485_conScene(&parsebuf[i]);
                    break;
                }

                case E_CON_AIR_SET:
                case E_CON_AIR_UPDATE:
                {
                    Iot_485_conAir(&parsebuf[i]);
                    break;
                }

                case E_CON_NEWAIR_SET:
                case E_CON_NEWAIR_UPDATE:
                {
                    Iot_485_conNewair(&parsebuf[i]);
                    break;
                }

                case E_CON_FLOOR_SET:
                case E_CON_FLOOR_UPDATE:
                {
                    Iot_485_conFloor(&parsebuf[i]);
                    break;
                }
            }
            j = parsebuf[i+2]+2+2;
        }
        else
        {
            break;
        }
    }
}

// 客厅北射灯
//recvbuf len=12:0xaa 0x20 0x3 0x0 0x60 0x0 0x2d
///U8 temp[8] = {0xaa, 0x30, 0x3, 0x0, 0x60, 0x1, 0x0, 0x0};
int Iot_485::count_test = 0;

BOOL Iot_485::Iot_485_masterSlaveComm(U8 *buf)
{
    IOT_COMM_INFO("Enter Iot_485_masterSlaveComm cmd = 0x%x, \n", buf[1]);
    if(buf[1] == E_CON_SLAVE_SEARCH)
    {
        //(flagScene == E_CON_DEV_SETDODING)  SCENE 没有反馈状态
        if((flagLight == E_CON_DEV_SETDODING)||
           (flagCurtain == E_CON_DEV_SETDODING)||
           (flagAir == E_CON_DEV_SETDODING) ||
           (flagFloor== E_CON_DEV_SETDODING)||
           (flagNewAir == E_CON_DEV_SETDODING))
        {
        /*
            //对轮寻 的cmd 进行回馈
            U8 tbuf[6] = {0xaa, 0x11, 0x1, 0x1,0x00,0x00};
            Iot_485_checkSum(tbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(tbuf);
            */
        }

        //对轮寻 的cmd 进行回馈
        U8 tbuf[6] = {0xaa, 0x11, 0x1, 0x1,0x00,0x00};
        Iot_485_checkSum(tbuf);
        usleep(5000);
        iotSock485->Iot_Sock_sendbuf(tbuf);

        //对接IOT 中控interface 接口
    }else if(buf[1] == E_CON_PERMIT_SLAVE)
    {
        // 对Light 数据的通信机型返回上
        //U8 temp1[8] = {0xaa, 0x30, 0x3, 0x0, 0x60, 0x0, 0x0, 0x0};

        //0xaa 0x30 0x3 0x0 0x53 0x0 0x86
        if(flagLight == E_CON_DEV_SETDODING)
       // if(0)
        {
            Iot_485_checkSum(lightbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(lightbuf);   //发送更新

            U8 temp2[6] = {0xaa, 0x13, 0x1, 0x1,0x00,0x00};
            Iot_485_checkSum(temp2);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(temp2);     // 发送更新完成
            flagLight = E_CON_DEV_SETDONE;

            count_test = 0;
            return TRUE;
        }
        // 对curtain 数据的通信机型返回上
        //U8 temp1[8] = {0xaa 0x31 0x3 0x0 0x70 0x1 0x3f};
        if(flagCurtain == E_CON_DEV_SETDODING)
        {
            Iot_485_checkSum(cutainbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(cutainbuf);

            ////U8 temp2[6] = {0xaa, 0x13, 0x1, 0x1,0x00,0x00};
            Iot_485_checkSum(cutainbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(cutainbuf);
            flagCurtain = E_CON_DEV_SETDONE;
            return TRUE;
        }

        // 对scene 数据的通信不返回状态
        /*
             //U8 temp1[8] = {0xaa 0x23 0x3 0x0 0x89 0x1 0x5a};
             if(flagScene == E_CON_DEV_SETDODING)
             {
                 Iot_485_checkSum(scenebuf);
                 usleep(5000);
                 iotSock485->Iot_Sock_sendbuf(scenebuf);

                 //U8 temp2[6] = {0xaa, 0x13, 0x1, 0x1,0x00,0x00};
                 Iot_485_checkSum(scenebuf);
                 usleep(5000);
                 iotSock485->Iot_Sock_sendbuf(scenebuf);
                 flagScene = E_CON_DEV_SETDONE;
              }
             */

         // 对air 数据的通信不返回状态
         //U8 temp1[10] = {0xaa 0x22 0x6 0x74	 0x1  0x0  0x1	 0x34 0x0  0x7c};
         if(flagAir == E_CON_DEV_SETDODING)
         {
             Iot_485_checkSum(airbuf);
             usleep(5000);
             iotSock485->Iot_Sock_sendbuf(airbuf);

             Iot_485_checkSum(airbuf);
             usleep(5000);
             iotSock485->Iot_Sock_sendbuf(airbuf);
             flagAir = E_CON_DEV_SETDONE;
             return TRUE;
         }

         // 对newair 数据的通信不返回状态
         //U8 temp1[8] = {0xaa 0x24 0x4 0x84 0x1 0x0 0x1 0x58};
         if(flagNewAir== E_CON_DEV_SETDODING)
         {
             Iot_485_checkSum(newairbuf);
             usleep(5000);
             iotSock485->Iot_Sock_sendbuf(newairbuf);

             Iot_485_checkSum(newairbuf);
             usleep(5000);
             iotSock485->Iot_Sock_sendbuf(newairbuf);
             flagNewAir = E_CON_DEV_SETDONE;
             return TRUE;
        }

        // 对floor 数据的通信不返回状态
        //U8 temp1[8] = {0xaa 0x25 0x3 0x94 0x1 0x34 0x9b};
        if(flagFloor== E_CON_DEV_SETDODING)
        {
            Iot_485_checkSum(floorbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(floorbuf);

            Iot_485_checkSum(floorbuf);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(floorbuf);
            flagFloor = E_CON_DEV_SETDONE;
            return TRUE;
        }

        if(count_test == 8)
        {
            U8 lightbuf1[8] = {0xaa, 0x30, 0x3, 0x0, 0x53, 0x1, 0x86};
            Iot_485_checkSum(lightbuf1);
            usleep(5000);
            iotSock485->Iot_Sock_sendbuf(lightbuf1);   //发送更新

            IOT_COMM_DEBUG("\n#############################\n");
            count_test = 0;
        }
        count_test++;

        U8 temp3[6] = {0xaa, 0x13, 0x1, 0x1,0x00,0x00};
        Iot_485_checkSum(temp3);
        usleep(5000);
        iotSock485->Iot_Sock_sendbuf(temp3);

        //对接IOT 中控interface 接口
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conLight(U8 *buf)
{
    IOT_COMM_DEBUG("\nEnter Iot_485_conLight addr=0x%x,0x%x, cmd = 0x%x\n", buf[3], buf[4], buf[1]);
    //0xaa 0x20 0x3 0x0 0x60 0x1 0x2e
    if(buf[1] == E_CON_LIGHT_SET||buf[1] == E_CON_LIGHT_UPDATE)
    {
        memcpy(lightbuf, buf,sizeof(char)*7);
        flagLight = E_CON_DEV_SETDODING;
        lightbuf[1] = E_CON_LIGHT_UPDATE;
        if(buf[5] == E_CON_LIGHT_ON)
        {
            lightbuf[5] = E_CON_LIGHT_ON;
            // Iot_485_checkSum(temp);
            //   usleep(5000);
            // iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_LIGHT_OFF)
        {
            lightbuf[5] = E_CON_LIGHT_OFF;
            // Iot_485_checkSum(temp);
            // usleep(5000);
            //iotSock485->Iot_Sock_sendbuf(temp);
        }

 
         //对接IOT 中控interface 接口setDevStatusToHB
//         if(buf[1] == E_CON_LIGHT_UPDATE) --->是有中控底层发送出的callback  更新devices 状态
//         ifaceContorl->setDevStatusToHB(((lightbuf[3]<<8)|lightbuf[4]), DEVICE_TYPE_LIGHT, lightbuf);
    }
    else if(buf[1] == E_CON_LIGHT_STATUS)
    {
        //对接IOT 中控interface 接口 getDevStatusToHB
    }

    return TRUE;
}

BOOL Iot_485::Iot_485_conCurtain(U8 *buf)
{
    IOT_COMM_DEBUG("\nEnter Iot_485_conCurtain addr=0x%x,0x%x, cmd = 0x%x\n", buf[3], buf[4], buf[1]);
    //0xaa 0x21 0x3 0x0 0x70 0x1 0x3f

    if(buf[1] == E_CON_CURTAIN_SET)
    {
        memcpy(cutainbuf, buf, sizeof(char)*7);
        flagCurtain = E_CON_DEV_SETDODING;
        cutainbuf[1] = E_CON_CURTAIN_UPDATE;
        if(buf[5] == E_CON_CURTAIN_ON)
        {
            cutainbuf[5] = E_CON_CURTAIN_ON;
            // Iot_485_checkSum(temp);
            //   usleep(5000);
            // iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_CURTAIN_OFF)
        {
            //cutainbuf[5] = E_CON_CURTAIN_OFF;
            cutainbuf[5] = E_CON_CURTAIN_OFF;
            // Iot_485_checkSum(temp);
            // usleep(5000);
            //iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_CURTAIN_STOP)
        {
            cutainbuf[5] = E_CON_CURTAIN_STOP;
        }

//        ifaceContorl->setDevStatusToHB(((lightbuf[3]<<8)|lightbuf[4]), DEVICE_TYPE_CURTAIN, cutainbuf);
        //对接IOT 中控interface 接口
    }
    else if(buf[1] == E_CON_CURTAIN_STATUS)
    {
        //对接IOT 中控interface 接口getDevStatusToHB
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conScene(U8 *buf)
{
    IOT_COMM_DEBUG("\nEnter Iot_485_conScene addr=0x%x, cmd = 0x%x\n", buf[3], buf[1]);
    //0xaa 0x23 0x3 0x0 0x90 0x1 0x61

    if(buf[1] == E_CON_SCENE_SET)
    {
        memcpy(scenebuf, buf, sizeof(char)*7);
        flagScene= E_CON_DEV_SETDODING;
        scenebuf[1] = E_CON_SCENE_STATUS;

        if(buf[5] == E_CON_SCENE_CALL)
        {
            scenebuf[5] = E_CON_SCENE_CALL;
            // Iot_485_checkSum(temp);
            //   usleep(5000);
            // iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_SCENE_LEARN)
        {
            //cutainbuf[5] = E_CON_CURTAIN_OFF;
            scenebuf[5] = E_CON_SCENE_LEARN;
            // Iot_485_checkSum(temp);
            // usleep(5000);
            //iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_SCENE_EXIT)
        {
            scenebuf[5] = E_CON_SCENE_EXIT;
        }
        //对接IOT 中控interface 接口
    }
    else if(buf[1] == E_CON_SCENE_STATUS)
    {
        //对接IOT 中控interface 接口
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conAir(U8 *buf)
{
    IOT_COMM_DEBUG("\nEnter Iot_485_conAir addr=0x%x, cmd = 0x%x\n", buf[3], buf[1]);
    //0xaa 0x22 0x6 0x74    0x1  0x0  0x1  0x34  0x0  0x7c
    //   0     1     2    3        4     5      6      7      8     9
    if(buf[1] == E_CON_AIR_SET)
    {
        memcpy(airbuf, buf, sizeof(char)*10);
        flagAir = E_CON_DEV_SETDODING;
        airbuf[1] = E_CON_AIR_UPDATE;

        if(buf[5] == E_CON_AIR_ON)
        {
            airbuf[2] = 0x07;
            airbuf[5] = E_CON_AIR_ON;
            airbuf[9] = airbuf[8];

            // 需要从温度传感器获取当前的温度 will doing function
            airbuf[8] = 0x1A;

            // Iot_485_checkSum(temp);
            // usleep(5000);
            // iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[5] == E_CON_AIR_OFF)
        {
            airbuf[2] = 0x07;
            airbuf[5] = E_CON_AIR_OFF;
            airbuf[9]	= airbuf[8];
            // 需要从温度传感器获取当前的温度 will doing function
            airbuf[8] = 0x1C;

            // Iot_485_checkSum(temp);
            // usleep(5000);
            //iotSock485->Iot_Sock_sendbuf(temp);
        }
        //对接IOT 中控interface 接口
    }
    else if(buf[1] == E_CON_AIR_UPDATE)
    {
        //对接IOT 中控interface 接口
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conFloor(U8 *buf)
{
    IOT_COMM_DEBUG("\nEnter Iot_485_conFloor addr=0x%x, cmd = 0x%x\n", buf[3], buf[1]);
    //0xaa 0x25 0x3 0x94 0x1 0x34 0x9b 0x00

    if(buf[1] == E_CON_FLOOR_SET)
    {
        memcpy(floorbuf, buf, sizeof(char)*7);
        flagFloor = E_CON_DEV_SETDODING;
        floorbuf[1] = E_CON_FLOOR_UPDATE;

        if(buf[4] == E_CON_FLOOR_ON)
        {
            floorbuf[2] = 0x04;
            floorbuf[4] = E_CON_FLOOR_ON;
            floorbuf[6] = floorbuf[5];

            // 需要从温度传感器获取当前的温度 will doing function
            // floorbuf[5] = 0x1A;

            // Iot_485_checkSum(temp);
            // usleep(5000);
            // iotSock485->Iot_Sock_sendbuf(temp);
        } else if(buf[4] == E_CON_FLOOR_OFF)
        {
            floorbuf[2] = 0x04;
            floorbuf[4] = E_CON_FLOOR_OFF;
            floorbuf[6] = floorbuf[5];

            // 需要从温度传感器获取当前的温度 will doing function
            ///floorbuf[5] = 0x1C;

            // Iot_485_checkSum(temp);
            // usleep(5000);
            //iotSock485->Iot_Sock_sendbuf(temp);
        }
        //对接IOT 中控interface 接口
    }
    else if(buf[1] == E_CON_AIR_UPDATE)
    {
        //对接IOT 中控interface 接口
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conNewair(U8 *buf)
{
    //0xaa 0x24 0x5 0x84 0x1 0x1 0x3 0xe7 0x43
    IOT_COMM_DEBUG("\nEnter Iot_485_conNewair addr=0x%x, cmd = 0x%x\n", buf[3], buf[1]);

    if(buf[1] == E_CON_NEWAIR_SET)
    {
        memcpy(newairbuf, buf, sizeof(char)*(4+buf[2]));
        flagNewAir = E_CON_DEV_SETDODING;
        newairbuf[1] = E_CON_NEWAIR_UPDATE;

        switch(newairbuf[5])
        {
        case E_NEWAIR_GETPM25:
            //接口需要实现
            newairbuf[6] = 0x01;
            newairbuf[7] = 0x01;
            break;

        case E_NEWAIR_GETCO2:
            //接口需要实现
            newairbuf[6] = 0x01;
            newairbuf[7] = 0x02;
            break;

        case E_NEWAIR_GETHCHO:
            //接口需要实现
            newairbuf[6] = 0x01;
            newairbuf[7] = 0x03;
            break;

        case E_NEWAIR_GETHUMIDITY:
            //接口需要实现
            newairbuf[6] = 0x00;
            newairbuf[7] = 0x04;
            break;
        }

        //对接IOT 中控interface 接口
    }
    else if(buf[1] == E_CON_NEWAIR_UPDATE)
    {
        //对接IOT 中控interface 接口
    }
    return TRUE;
}

BOOL Iot_485::Iot_485_conTempElec(U8 *buf)
{
    IOT_COMM_INFO("Enter Iot_485_conTempElec\n");
    return TRUE;
}

BOOL Iot_485::Iot_485_conTempMot(U8 *buf)
{
    IOT_COMM_INFO("Enter Iot_485_conTempMot\n");
    return TRUE;
}
