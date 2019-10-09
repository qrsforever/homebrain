#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define HELLO_PORT 1900
#define HELLO_GROUP "239.255.255.250"
#define MSGBUFSIZE 256
#define JSON_MAX_LEN 1401

char g_hostIP[MSGBUFSIZE] = { 0 };
char g_SN[JSON_MAX_LEN] = { 0 };

char *network_inf = "eth0";
char *json_file_name = "hostinfo.txt";

void dump_to_hex(char *buf)
{
	int i;
	printf("\n");
	int len = strlen(buf);
	for (i = 0; i < len; i++)
		printf("%02X", buf[i]);
	printf("\n");
}

int get_ip(const char *ifname, char *ip)
{
	int sock_get_ip;
	int s32Ret = 0;
	char ipaddr[48];
	struct sockaddr_in *sin;
	struct ifreq ifr;

	if ((sock_get_ip = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("socket create failed!\r\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name, ifname, strlen(ifname));

	if (ioctl(sock_get_ip, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl error!\n");
		s32Ret = -2;
		goto FAILED;
	}

	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	if (NULL != ip)
		strcpy(ip, inet_ntoa(sin->sin_addr));

FAILED:
	close(sock_get_ip);
	return s32Ret;
}

int open_and_read_file(char *filename, char *buf)
{
	int fd, n;
	fd = open(filename, O_RDONLY);
	n = read(fd, buf, JSON_MAX_LEN);
	printf("the content is %s,%d bytes\n", buf, n);
	close(fd);
	return n;
}

void do_process_search(char *buf, int buflen, int socket,
		       struct sockaddr_in *addr, int addrlen)
{
	time_t now;
	char resp_buffer[JSON_MAX_LEN - 1] = { 0 };

	/* get host IP address */
	memset(g_hostIP, 0, MSGBUFSIZE);
	memset(g_SN, 0, sizeof(g_SN));
	if (get_ip(network_inf, g_hostIP) < 0) {
		return;
	}
	if (open_and_read_file(json_file_name, g_SN) <= 0) {
		return;
	}

	now = time(NULL);
	snprintf(resp_buffer, JSON_MAX_LEN,
		 "{\"stamp\":\"%ld\", \"location\":\"%s\", \"server\":\"LeHB 1.0\", \"st\":\"lehb\", %s}",
		 now, g_hostIP, g_SN);
#if 0
	snprintf(resp_buffer, 1024,
		 "{\"stamp\":\"%ld\", \"location\":\"%s\", \"server\":\"LeHB 1.0\", \"st\":\"lehb\", \"usn\":\"%s\", \"sid\": \"%s\"}",
		 now, g_hostIP, g_SN, g_SN);
#endif
	int result = sendto(socket, resp_buffer, strlen(resp_buffer), 0,
			    (struct sockaddr *)addr, addrlen);
	printf("resp: \n%s\n", resp_buffer);
	printf("Send response (%d)\n", result);
}

main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int fd, nbytes, addrlen;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];
	u_int yes = 1;	     /*** MODIFICATION TO ORIGINAL */

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		exit(1);
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	/* N.B.: differs from sender */
	addr.sin_port = htons(HELLO_PORT);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}
	while (1) {
		//system("route add -net 239.255.255.250 netmask 255.255.255.255 eth0");
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt
		    (fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
		     sizeof(mreq)) < 0) {
			perror("setsockopt");
			sleep(1);
			//exit(1);
		} else
			break;
	}

	/* now just enter a read-print loop */
	while (1) {
		addrlen = sizeof(addr);
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0,
				       (struct sockaddr *)&addr,
				       &addrlen)) < 0) {
			perror("recvfrom");
			exit(1);
		}
		msgbuf[nbytes] = 0;
		if (strncmp(msgbuf, "M-SEARCH * lehb", 15) == 0) {
			struct sockaddr_in mysock = addr;
			printf("%s", inet_ntoa(mysock.sin_addr));
			//puts(msgbuf);
			do_process_search(msgbuf, nbytes, fd, &addr, addrlen);
		}
	}
}
