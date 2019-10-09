#ifndef _IOTSOCK_H_
#define _IOTSOCK_H_
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

typedef enum
{
    E_RECV = 0,
    E_SEND = 1,
}EN_RECV_SEND;

class Iot_Sock
{
public:
    Iot_Sock();
    virtual ~Iot_Sock();
    BOOL Iot_Sock_setup(const INT port, const U8 *netaddr);
    void Iot_Sock_select();
    void Iot_Sock_recvbuf(U8 *recvbuf);
    BOOL Iot_Sock_sendbuf(U8 *sendbuf);
public :
    struct sockaddr_in servaddr;
    struct timeval trecv;
    struct timeval tsend;
    EN_RECV_SEND rscon;  //控制发送或接收的标志

    INT sock_cli;
    fd_set rfds;
    INT retval;
    INT maxfd;
    INT lenbuf;
};
#endif
