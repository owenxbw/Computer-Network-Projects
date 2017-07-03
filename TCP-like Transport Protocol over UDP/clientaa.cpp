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
#include <map>
#include "TCPHeader.h"
using namespace std;
#define PORT 1160
#define BUFSIZE 2048
#define HEADER_LEN 8

int
main(int argc, char* argv[])
{
	int inputNum= argc;
  // int ClientACK=0;
  // int ServerACK=0;
	int recvlen; 
    char header[8]={"hellllo"};
	char buf[BUFSIZE];       //unsigned char vs. char??????????
	// struct sockaddr_in myaddr; 
	struct sockaddr_in serverAddr;


	std::map<int,string> buffer;
	std::map<int,string>::iterator it;
	std::map<int,string>::iterator at;
	string::size_type pos = 0;
	string bufstr;




	socklen_t addrlen = sizeof(serverAddr); 
	// string hostname[inputNum];     //why name[i]?
	// string filename[inputNum];
	for (int i = 0; i < inputNum; i++) {
		cout << "The " << i << " th Input string is :" << argv[i] << endl; 
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //int socket(int family, int type, int protocol);

//claim server
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(4000);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr("10.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  //initiate TCPheade

	// const char* msg = "Jane Doe";

	size_t header_len = strlen(header);
  //size_t msg_length = header.size();
	// size_t tcpmsg_length = tcpmsg.size();

	int result = sendto(sockfd, header, header_len, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	// int result2 =sendto(sockfd, tcpmsg.c_str(), tcpmsg_length, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));   //c_str!!!

	std::cout << result << " bytes sent" << std::endl;
	// std::cout << result2 << " bytes sent" << std::endl;

	for(int i=1;i<10;i++){
	// only makes loop three times
    recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);
    printf("received %d bytes\n\n", recvlen);
    if (recvlen > 0) {
            buf[recvlen] = 0;
            bufstr.assign(buf,recvlen);
            buffer.insert (std::pair<int,string>(i,bufstr.substr(pos+8)));
            std::cout<<"received "<<i<<" th message:"<<"\n"<<buf<<endl;
            sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
     }

 }


  // at = buffer.find('1');
  // if (at != buffer.end())
  //   buffer.erase (at);

 		// buffer.erase(2);                                        //erase(key)

	// at=buffer.find(3);

	std::cout <<"3 is"<< buffer.find(3)-> second << '\n';            //right

 		std:: ofstream outfile;
 		outfile.open("received.txt",ios::out);   //saving data to "received data" file; app:append mode
	    outfile << buffer.find(3)-> second;
	    outfile.close();

	    for(int i=4;i<7;i++){
	            it = buffer.find(i);  //find next pkt in buffer
	            if(it!=buffer.end()){
		         buffer.erase(i);
		         std::cout << " Deleted   " << i << '\n';

	             }else{break;}   }

	    std::cout << "buffer contains:\n";
	  	for (it=buffer.begin(); it!=buffer.end(); ++it){
	    std::cout << it->first << " => " << it->second << '\n';
	    std::cout <<  " with size of  " << (it->second).size() << '\n';
		}

	return 0;

}
