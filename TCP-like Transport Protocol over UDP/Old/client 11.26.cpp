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
#define SERVER_PORT 4000
#define BUFSIZE 2048
#define HEADER_LEN 8

int
main(int argc, char* argv[])
{
  int inputNum= argc;
  unsigned int ClientACK=0;
  int recvlen; 
  char header[8]={0};
  char buf[BUFSIZE];       //unsigned char vs. char??????????
  //struct sockaddr_in myaddr; 
  struct sockaddr_in serverAddr;
  socklen_t addrlen = sizeof(serverAddr); 

  for (int i = 0; i < inputNum; i++) {
    cout << "The " << i << " th Input string is :" << argv[i] << "\n\n"<<endl; 
  }

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //int socket(int family, int type, int protocol);
//claim server
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);    
  serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  TCPHeader ClientFirst;                //initiate TCPheader
  srand((unsigned)time(0));                                 //input a different seed
  unsigned short ClientSeq = rand()% 65536;                 //generate a random num smaller than 2bytes.
  ClientFirst.setSeqNumber(ClientSeq);
  ClientFirst.setAckNumber(0);
  ClientFirst.setWinSize(0);
  ClientFirst.setFlags(6);
  ClientFirst.generate(header);
  std::cout<<"Initial Client Sequence Num: "<< ClientSeq<<endl;
  std::cout<<"Sending packet: 0       SYN"<<endl;
  sendto(sockfd, header, HEADER_LEN, 0, ( sockaddr*)&serverAddr, sizeof(serverAddr));          //send handshake

for(int i=1;;i++){
    recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);     //receive header
    // recvlen = recv(sockfd, buf, BUFSIZE, 0);
    printf("\nreceived %d bytes\n", recvlen);
    if (recvlen > 0) {
            buf[recvlen] = 0;
            //receive header from server
            TCPHeader receiveACK;
            memcpy(header,buf,8);                                                   //memcpy
            //strcpy(header,buf);
            receiveACK.consume(header);
            unsigned short seqnum = receiveACK.seqNum();
            unsigned short acknum = receiveACK.ackNum();
            unsigned short winsize = receiveACK.winSize();
            unsigned short flag = receiveACK.flags();
            std::cout<<"Receiving packet: "<< seqnum<<endl;

            if(acknum==ClientSeq+1){                                    // if received in-order-packet

              if (flag==4){                                     // receive data content

              recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);        //receive cotent of file
          
                if (recvlen > 0) {
                  buf[recvlen] = 0;
              //receive header from server
                  std::ofstream outfile("received.data");               //create file
                  outfile << buf;
                  }
              }

              else if(flag==5){                        //FIN
                //FIN,break first?
              }

              //generate reply header
              ClientACK = seqnum+1;
              ClientSeq = acknum;
              TCPHeader ClientFollow;
              ClientFollow.setSeqNumber(ClientSeq);
              ClientFollow.setAckNumber(ClientACK);
              ClientFollow.setWinSize(winsize);
              ClientFollow.setFlags(4);                      
              ClientFollow.generate(header);  
              sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));  
              std::cout<<"Sending packet: "<< ClientACK<<endl;
          }

          else{
            std::cout << "Received out-of-order packet with Sequence Num: " << seqnum<< endl;
            continue;
          }
     }
}
  return 0;
}
