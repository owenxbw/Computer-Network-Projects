#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "TCPHeader.h"
using namespace std;
#define PORT 4000
#define BUFSIZE 2048

int
main(int argc, char **argv)
{
	struct sockaddr_in myaddr;      /* our address */
        struct sockaddr_in clientAddr;     /* remote address */
        socklen_t addrlen = sizeof(clientAddr);  /* length of addresses */
        int recvlen;                    /* # bytes received */
        int sockfd;                         /* our socket */
        char buf[BUFSIZE];     /* receive buffer */
        char header[8]={0};

        /* create a UDP socket */

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("cannot create socket\n");
                return 0;
        }

        /* bind the socket to any valid IP address and a specific port */

        memset((char *)&myaddr, 0, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(PORT);

        if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
                perror("bind failed");
                return 0;
        }

        //claim client
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = htons(1160);     // short, network byte order
        clientAddr.sin_addr.s_addr = inet_addr("10.0.0.2");
        memset(clientAddr.sin_zero, '\0', sizeof(clientAddr.sin_zero));





        const char* msg = "ACK back";
        size_t msg_length = strlen(msg);

        /* now loop, receiving data and printing what we received */
        for (;;) {
                printf("waiting on port %d\n", PORT);
                recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &addrlen);
                TCPHeader bbb;
                strcpy(header,buf);
                bbb.consume(header);
                printf("received %d bytes\n", recvlen);
                unsigned short seqnum = bbb.seqNum();
                unsigned short acknum = bbb.ackNum();
                unsigned short winsize = bbb.winSize();
                unsigned short flag = bbb.flags();
                cout << "Sequence Number is: " << seqnum << endl;
                cout << "Acknowledge Number is: " << acknum << endl;
                cout << "Window Size is: " << winsize << endl;
                cout << "Flags is: " << flag << endl;
                if (recvlen > 0) {
                        buf[recvlen] = 0;
                        printf("received message: \"%s\"\n", buf);
                        //return ack when receives msg
                        sendto(sockfd, msg, msg_length, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                }
        }
        /* never exits */
}
