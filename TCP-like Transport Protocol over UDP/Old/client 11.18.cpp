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
#define PORT 1160
#define BUFSIZE 2048
#define HEADER_LEN 8

int
main(int argc, char* argv[])
{
	int inputNum= argc;
  int ClientACK=0;
  int ServerACK=0;
	int recvlen; 
  char header[8]={0};
	char buf[BUFSIZE];       //unsigned char vs. char??????????
	struct sockaddr_in myaddr; 
	struct sockaddr_in serverAddr;
	socklen_t addrlen = sizeof(serverAddr); 
	// string hostname[inputNum];     //why name[i]?
	// string filename[inputNum];
	for (int i = 0; i < inputNum; i++) {
		cout << "The " << i << " th Input string is :" << argv[i] << endl; 
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //int socket(int family, int type, int protocol);

/* bind the socket to any valid IP address and a specific port */

        memset((char *)&myaddr, 0, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(PORT);
//bind client
        if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
                perror("bind failed");
                return 0;
        }

//claim server
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1153);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  //initiate TCPheader
  TCPHeader ClientFirst;
  int Clientseq= rand()* rand() % 65536;
  ClientFirst.setSeqNumber(Clientseq);
  ClientFirst.setAckNumber(0);
  ClientFirst.setWinSize(0);
  ClientFirst.setFlags(6);
  ClientFirst.generate(header);

	// const char* msg = "Jane Doe";

	size_t header_len = strlen(header);
  //size_t msg_length = header.size();
	// size_t tcpmsg_length = tcpmsg.size();

	int result = sendto(sockfd, header, header_len, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	// int result2 =sendto(sockfd, tcpmsg.c_str(), tcpmsg_length, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));   //c_str!!!

	std::cout << result << " bytes sent" << std::endl;
	// std::cout << result2 << " bytes sent" << std::endl;

	for(int i=1;i<3;i++){
	// only makes loop three times
    recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);
    printf("received %d bytes\n\n", recvlen);
    if (recvlen > 0) {
            buf[recvlen] = 0;

            TCPHeader receiveACK;
            strcpy(header,buf);
            receiveACK.consume(header);
            printf("received %d bytes\n", recvlen);
            unsigned short seqnum = receiveACK.seqNum();
            unsigned short acknum = receiveACK.ackNum();
            unsigned short winsize = receiveACK.winSize();
            unsigned short flag = receiveACK.flags();
            ClientACK = sequm+1;
            ServerACK = acknum;
            cout << "Sequence Number is: " << seqnum << endl;
            cout << "Acknowledge Number is: " << acknum << endl;

            TCPHeader ClientFollow;
            ClientFollow.setSeqNumber(++Clientseq);
            ClientFollow.setAckNumber(ClientACK+1);
            ClientFollow.setWinSize(0);
            ClientFollow.setFlags(6);
            ClientFollow.generate(header);

            std::cout<<"received "<<i<<" th message:"<<"\n"<<buf<<endl;
            int result = sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
     }
 }

	return 0;

}
