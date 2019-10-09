#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>

#include "common.h"
#include "iotSock.h"
#include "iot485.h"

int main_temp()
{
    U8 rbuf[BUFFER_SIZE];
    U8 sbuf[BUFFER_SIZE];
    const U8 *netaddr = NETADDR;
    const INT port = PORT;
    INT ret;
    memset(rbuf, 0, sizeof(rbuf));
    memset(sbuf, 0, sizeof(sbuf));

    Iot_485 *iot485 = new Iot_485();
    Iot_Sock *iotScok = new Iot_Sock();
    ret = iotScok->Iot_Sock_setup(port, netaddr);
    //Sock setup fail will be retry again
    while(ret != TRUE) {
        ret = iotScok->Iot_Sock_setup(port, netaddr);
        IOT_COMM_DEBUG("sock connect fail, sleep 1s will retry again!\n");
        sleep(1);
    }

    while(1) {
        iotScok->Iot_Sock_select();
        if(iotScok->retval == -1) {
            IOT_COMM_DEBUG("select erro client exit \n");
            break;
        } else if(iotScok->retval == 0) {
            //IOT_COMM_DEBUG("Recv buf no message in client waiting...\n");
            continue;
        } else {
            //U8 temp[5] = {0xaa, 0x11, 0x1, 0x1,0xbd};
            //iotScok->Iot_Sock_sendbuf(temp);
            iotScok->Iot_Sock_recvbuf(rbuf);
            //输出获取到的buf
            if (iotScok->lenbuf !=0 ){
                IOT_COMM_DEBUG("\nrecvbuf len=%d:", iotScok->lenbuf);
                for(INT i = 0; i<iotScok->lenbuf; i++) {
                    IOT_COMM_DEBUG("0x%x ", rbuf[i]);
                }
                IOT_COMM_DEBUG("\n");

                iot485->iotSock485 = iotScok;
                iot485->Iot_485_parseCmd(rbuf, iotScok->lenbuf);
            }
        }
        memset(rbuf, 0, sizeof(rbuf));
    }

    delete iotScok;
    delete iot485;
    return 0;
}
