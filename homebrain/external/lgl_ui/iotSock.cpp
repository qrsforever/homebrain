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
#include "common.h"
#include "iotSock.h"

Iot_Sock::Iot_Sock()
{
    IOT_COMM_INFO("Iot_Sock construct\n");

    memset(&servaddr, 0, sizeof(servaddr));
    trecv.tv_sec = 5;
    trecv.tv_usec = 0;
    tsend.tv_sec = 0;
    tsend.tv_usec = 0;

    FD_ZERO(&rfds);
    maxfd = 0;
    lenbuf = 0;
    rscon = E_RECV;
}

Iot_Sock::~Iot_Sock()
{
    IOT_COMM_INFO("~Iot_Sock deconstruct\n");

    FD_ZERO(&rfds);
    maxfd = 0;
    close(sock_cli);
}

BOOL Iot_Sock::Iot_Sock_setup(const INT port, const U8* netaddr)
{
    IOT_COMM_INFO("Enter Iot_Sock_setup\n");
    sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(netaddr);
    IOT_COMM_DEBUG("IP_ADDR:PORT=%s:%d", inet_ntoa(servaddr.sin_addr), port);

    while (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        IOT_COMM_DEBUG("Iot_Sock_setup connect fail!\n");
        return FALSE;
    }

    IOT_COMM_INFO("Enter Iot_Sock_setup sock_cli = %d\n", sock_cli);
    return TRUE;
}

void Iot_Sock::Iot_Sock_select()
{
    IOT_COMM_INFO("\nEnter Iot_Sock_select sock_cli = %d\n",sock_cli);

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    maxfd = 0;
    FD_SET(sock_cli, &rfds);
    if(maxfd < sock_cli) {
        maxfd = sock_cli;
    }
    retval = select(maxfd+1, &rfds, NULL, NULL, &trecv);
}

void Iot_Sock::Iot_Sock_recvbuf(U8 *recvbuf)
{
    lenbuf = 0;
    if(FD_ISSET(sock_cli, &rfds)) {
        IOT_COMM_INFO("Enter Iot_Sock_recvbuf recvbuf= %p\n", recvbuf);
        lenbuf = recv(sock_cli, recvbuf, sizeof(U8)*BUFFER_SIZE, 0);
    }
}

BOOL Iot_Sock::Iot_Sock_sendbuf(U8 *sendbuf)
{
    //IOT_COMM_DEBUG("\nEnter Iot_Sock_sendbuf sock_cli= %d\n", sock_cli);

    INT len = sendbuf[2]+4;

    IOT_COMM_DEBUG("sendbuf len=%d, sock_cli= %d:", len, sock_cli);
    for(INT i = 0; i<len; i++) {
        IOT_COMM_DEBUG("0x%x ", sendbuf[i]);
    }
    IOT_COMM_DEBUG("\n");

    if(FD_ISSET(sock_cli, &rfds)) {
       /// IOT_COMM_DEBUG("enter sendbuf send=----xf.wang=--\n");
        send(sock_cli, sendbuf, len, 0);
        memset(sendbuf, 0x0, len+1);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

