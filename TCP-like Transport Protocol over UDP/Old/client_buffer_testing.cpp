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
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include "TCPHeader.h"
using namespace std;
#define SERVER_PORT 4000
#define BUFSIZE 15360
#define HEADER_LEN 8
#define MAXSEQNUM 30720

int
main(int argc, char* argv[])
{
    int inputNum= argc;
    int countPkt = 0;
    unsigned long ClientACK=0;
    unsigned long savedACK = 0;
    bool SYNACK = true;
    bool FINACK = false;
    bool Retransmission = false;
    int recvlen;
    string bufstr;
    string::size_type pos = 0;
    char header[8]={0};
    char buf[BUFSIZE];       //unsigned char vs. char??????????
	std::map<int,string> buffer;    //buffer to store out-of-order packets<seq num,data,datalen>
	std::map<int,string>::iterator it= buffer.begin();


    std:: ofstream outfile;   // create a file
 //    outfile.open("received.data",ios::out);   //saving data to "received data" file; app:append mode
	// outfile << ;
	// outfile.close();


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
    
    /* Three way handshake for SYN */
    TCPHeader ClientFirst;                //initiate TCPheader
    srand((unsigned)time(0));                                 //input a different seed
    // unsigned short ClientSeq = 1;
    unsigned long ClientSeq = rand()% 30720;                 //generate a random num smaller than 2bytes.
    ClientFirst.setSeqNumber(ClientSeq);
    ClientFirst.setAckNumber(0);
    ClientFirst.setWinSize(0);
    ClientFirst.setFlags(6);
    ClientFirst.generate(header);
    std::cout<<"Initial Client Sequence Num: "<< ClientSeq<<endl;
    std::cout<<"Sending packet 0       SYN"<<endl;
    sendto(sockfd, header, HEADER_LEN, 0, ( sockaddr*)&serverAddr, sizeof(serverAddr));          //send handshake


    while(1){
        recvlen = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&serverAddr, &addrlen);     //receive header

        // recvlen = recv(sockfd, buf, BUFSIZE, 0);
        // printf("\nreceived %d bytes\n", recvlen);
	        if (recvlen > 0) {
	            buf[recvlen] = 0;

	            //receive header from server
	            TCPHeader receiveACK;
	            memcpy(header,buf,8);                                                   //memcpy
	            bufstr.assign(buf, recvlen);   // copy buf to string
	            //strcpy(header,buf);
	            receiveACK.consume(header);
	            unsigned short seqnum = receiveACK.seqNum();
	            unsigned short acknum = receiveACK.ackNum();
	            unsigned short winsize = 5000;
	            // unsigned short flag = receiveACK.flags();
	           
	            // printf("flag = %d\n", flag);
	            std::cout<<"Receiving packet "<< seqnum<<" ,with size "<<recvlen<< " bytes"<<endl;
	            
	        // Check if received in-order-packet
	        if(seqnum == savedACK || SYNACK || FINACK) {
	            if(receiveACK.SYN() & receiveACK.ACK()) {  // ASF = 110
	                //generate reply header
	                ClientACK = seqnum+1;
	                ClientSeq = acknum;
	                TCPHeader ClientFollow;
	                ClientFollow.setSeqNumber(ClientSeq);
	                ClientFollow.setAckNumber(ClientACK);
	                ClientFollow.setWinSize(winsize);
	                ClientFollow.setFlags(6);     // SYN-ACK
	                ClientFollow.generate(header);
	                sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	                savedACK = ClientFollow.ackNum();
	                printf("%s\n", "Connection Established");
	                std::cout<<"Sending packet "<< ClientACK<<endl;
	                SYNACK = false;


	            } else if(receiveACK.FIN()) {   // ASF = 001
	                if(receiveACK.ACK()) {   // ASF = 101
	                    printf("%s\n", "Connection Close");
	                    close(sockfd);
	                    break;
	                } else if(receiveACK.SYN()) {   // ASF = 011
	                    printf("%s\n", "Error: SYN() and FIN() cannot be happened at the same time");
	                } else {
	                    //generate reply header
	                    ClientACK = seqnum+1;
	                    ClientSeq = acknum;
	                    TCPHeader ClientFollow;
	                    ClientFollow.setSeqNumber(ClientSeq);
	                    ClientFollow.setAckNumber(ClientACK);
	                    ClientFollow.setWinSize(winsize);
	                    ClientFollow.setFlags(4);    // ACK
	                    ClientFollow.generate(header);
	                    sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	                    std::cout<<"Sending packet "<< ClientACK<<endl;

	                    // Send FIN to Server
	                    TCPHeader ClientFin;
	                    ClientFin.setAckNumber(0);
	                    ClientFin.setWinSize(winsize);
	                    ClientFin.setFlags(1);   // FIN
	                    ClientFin.generate(header);
	                    sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	                    savedACK = ClientFin.ackNum();
	                    // "Sending packet" [ACK number] ("Retransmission") ("SYN") ("FIN")
	                    printf("Sending packet %d FIN\n", ClientFin.ackNum());
	                    FINACK = true;
	                }
	                
	            } else {   // ASF = 000
	                //generate reply header      //when Retransmission is false

	                if (countPkt == 0) {
	                	// For the first data packet
	                	outfile.open("received.data",ios::out);   //saving data to "received data" file; app:append mode
	                	outfile << bufstr.substr(pos+8);
	                	outfile.close();
	                	printf("%s\n", "The first Data is saved.");
	                	countPkt++;
	                	seqnum = seqnum+recvlen-8;  // should be incremented by the size of data
		                seqnum %= 30721;     //if seqnum goes over 30721
		            	savedACK=seqnum;
	                }else{

	                // } else {
	                // 	outfile.open("received.data",ios::app);   //saving data to "received data" file; app:append mode
	                // 	outfile << bufstr.substr(pos+8);
	                // 	outfile.close();
	                // 	// printf("%s\n", "Data is saved.");
	    
	                countPkt++;
	                printf("In-order Pkt Count = %d\n", countPkt);

	           		buffer.insert(std::pair<int,string>(seqnum,bufstr.substr(pos+8))); //store out-of-order pkt to buffer
       				std::cout << "buffer.size() is " << buffer.size() << '\n';
		            // seqnum = seqnum+recvlen-8;  // should be incremented by the size of data
		            // seqnum= seqnum % 30721;     //if seqnum goes over 30721
		            // savedACK=seqnum;            
		                while(1){
	                		it = buffer.find(seqnum);  //find next pkt in buffer
	                		if(it!=buffer.end()){
	                			outfile.open("received.data",ios::app);   //saving just received pkt
		                		outfile << it->second;
		                		outfile.close();
		                		seqnum+=(it->second).size();  //update seqnum to next pkt  
		                		seqnum= seqnum % 30721;         // if seqnum goes over 30721
		                		buffer.erase(savedACK);
		                		std::cout<<"Pushed and erased packet "<< savedACK<<endl;
		                		std::cout << "buffer.size() is " << buffer.size() << '\n';
		                		savedACK=seqnum;
	                		}else{break;}   //if it points to end of buffer
	                	}
	                }
	            
		                ClientSeq = acknum;
		                TCPHeader ClientFollow;
		                ClientFollow.setSeqNumber(ClientSeq);
		                ClientFollow.setAckNumber(seqnum);
		                ClientFollow.setWinSize(winsize);
		                ClientFollow.setFlags(4);     // ACK only
		                ClientFollow.generate(header);
		                sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		                savedACK = ClientFollow.ackNum();
		               	std::cout<<"Sending packet "<< seqnum<<endl;
	            }

       			
   			}
   		}
   	}
    return 0;
}

	           //      //when in-order packet is received after receiving out-of-order
	           //  	else {
	           //  		if (countPkt == 0) {
	           //      	// For the first data packet
	           //      	outfile.open("received.data",ios::out);   //saving data to "received data" file; app:append mode
	           //      	outfile << bufstr.substr(pos+8);
	           //      	outfile.close();
	           //      	printf("%s\n", "The first Data is saved.");
	           //     		} else {
	           //      	outfile.open("received.data",ios::app);   //saving data to "received data" file; app:append mode
	           //      	outfile << bufstr.substr(pos+8);
	           //      	outfile.close();
	           //     		}

		          //       seqnum += recvlen-8;
		          //       seqnum %= 30721; 
		          //       savedACK=seqnum;
	           //      	//push all the sequential pkts in buffer to file
	           //      	while(1){
	           //      		it = buffer.find(seqnum);  //find next pkt in buffer
	           //      		if(it!=buffer.end()){
	           //      			outfile.open("received.data",ios::app);   //saving just received pkt
		          //       		outfile << it->second;
		          //       		outfile.close();
		          //       		seqnum+=(it->second).size();  //update seqnum to next pkt  
		          //       		seqnum %= 30721;         // if seqnum wraps around
		          //       		buffer.erase(savedACK);
		          //       		std::cout<<"Pushed and erased packet "<< savedACK<<endl;
		          //       		savedACK=seqnum;
	           //      		}else{break;}   //if it points to end of buffer
	           //      	}
	           //      	std::cout << "buffer.size() is " << buffer.size() << '\n';
	           //      	//send accumulative ACK
	           //      	ClientACK = seqnum;                            //might be meaningless to use ClientACK???????
	           //      	ClientSeq = acknum;
		          //       TCPHeader ClientFollow;
		          //       ClientFollow.setSeqNumber(ClientSeq);
		          //       ClientFollow.setAckNumber(ClientACK);
		          //       ClientFollow.setWinSize(winsize);
		          //       ClientFollow.setFlags(4);     // ACK only
		          //       ClientFollow.generate(header);
		          //       sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		          //       savedACK = ClientFollow.ackNum();
		          //       std::cout<<"Sending packet "<< savedACK<<endl;
		          //       Retransmission = false;

	           //      } 

	           //  }
            // } else {
            // 	// Out-of-order packet is received 
            // 	printf("%s\n", "Out-of-order");
            // 	Retransmission = true; 



            // 	ClientACK = savedACK;
            // 	ClientSeq = acknum;
            // 	TCPHeader LostPacket;
            // 	LostPacket.setSeqNumber(ClientSeq);
            // 	LostPacket.setAckNumber(ClientACK);
            // 	LostPacket.setWinSize(winsize);
            // 	LostPacket.setFlags(4);   // ACK
            // 	LostPacket.generate(header);
            // 	sendto(sockfd, header,HEADER_LEN, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
            // 	std::cout<<"Sending packet "<< ClientACK<<"      Retransmission\n"<< endl;  //???to discuss
            // }

            