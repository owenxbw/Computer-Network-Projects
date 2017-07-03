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
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <sys/time.h>
#include <signal.h>
#include"TCPHeader.h"
using namespace std;

#define BUFSIZE 1024
#define PKTSIZE 1024
#define RTO 500

unsigned long currentTimeMs()
{
    timeval tv;
    gettimeofday(&tv, NULL);
  int long a = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  return a;
}

void closeConnection(unsigned long filesize, unsigned long myInitialSeq, int sockfd, sockaddr_in clientAddr,unsigned long& cwnd, unsigned long& ssthresh, sockaddr_in fclientAddr);
void sendFile(FILE* fp, unsigned long filesize, unsigned long myInitialSeq, unsigned short wdSize, int sockfd, sockaddr_in clientAddr, unsigned long& cwnd, unsigned long& ssthresh, sockaddr_in fclientAddr);

void fillThePkt(char* packet, unsigned length, unsigned long init, int pktNo, FILE* fp)
{
    TCPHeader HeaderObj;
    memset(packet, '\0', PKTSIZE+8);
    char header[8] = {0};
    HeaderObj.setSeqNumber((init + (unsigned long)pktNo*PKTSIZE));
    HeaderObj.generate(header);
    memcpy(packet, header, 8);
    fseek(fp, pktNo*PKTSIZE, SEEK_SET);
    if(fread(&packet[8],1,length,fp) != length)
        {cout << length << endl; cerr << " file read error!" << endl; exit(0);}
}

int main(int argc, const char **argv)
{
    if(argc == 1)
  {
    argv[1] = "4000";
    argv[2] = "./test.txt";
  }
  else if(argc != 3)
  {
    cerr<<"2 arguments required!"<<endl;
    return 1;
  }

  struct sockaddr_in myaddr;      /* our address */


  struct sockaddr_in fclientAddr;     /* remote address */
  socklen_t faddrlen = sizeof(fclientAddr);            /* length of addresses */
  int recvlen = 0;                    /* # bytes received */
  int sockfd = -1;                         /* our socket */
    unsigned short clientSeq = 0;
    char buf[BUFSIZE];     /* receive buffer */
    char headerToSend[8] = {0};
    char recvHeader[8] = {0};

  /* create a UDP socket */
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
    perror("cannot create socket\n");
    return 0;
  }

  /* bind the socket to any valid IP address and a specific port */
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(atoi(argv[1]));

  if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
    perror("bind failed");
    return 0;
  }

    //open file
    FILE * fp = fopen(argv[2],"r");
    if(NULL == fp )
    {
            printf("File:\t%s Not Found\n", argv[2]);
            return -4;
    }
    //read file size
    unsigned long filesize = 0;
    fseek(fp,0,SEEK_END);
    filesize = ftell(fp);
    fseek(fp,0,SEEK_SET);

    map<unsigned long, set<unsigned short>> table;
    signal(SIGCHLD, SIG_IGN);
  //repeatedly trying to connect
  while(1)
    {
    //printf("\n===> waiting on port %s\n", argv[1]);
    recvlen = recvfrom(sockfd, buf, 8, MSG_PEEK, (struct sockaddr *)&fclientAddr, &faddrlen);
        if (recvlen == 8)
        {
        char fipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(fclientAddr.sin_family, &fclientAddr.sin_addr, fipstr, sizeof(fipstr));

        if(!((table.find(fclientAddr.sin_addr.s_addr)!=table.end()) && (table[fclientAddr.sin_addr.s_addr].count(fclientAddr.sin_port) == 1))){
            recvlen = recvfrom(sockfd, buf, 8, 0, (struct sockaddr *)&fclientAddr, &faddrlen);
            std::cout << endl << "===> connection from: " << fipstr << ":" <<ntohs(fclientAddr.sin_port)
            <<" "<< fclientAddr.sin_addr.s_addr <<std::endl;
            table[fclientAddr.sin_addr.s_addr].insert(fclientAddr.sin_port);
            if(!fork())
            {
            struct sockaddr_in clientAddr;     /* remote address */
            socklen_t addrlen = sizeof(clientAddr);            /* length of addresses */
            TCPHeader FirstCall;
            FirstCall.consume(buf);
            if(FirstCall.flags() != 2)
                {cout << "Received Non-SYN signal! Going back to listening" << endl;cout << FirstCall.flags() << endl;_exit(0);}
            else
                {
                    clientSeq = FirstCall.seqNum();
                    cout << "Receiving packet " << FirstCall.ackNum() << endl;
                    TCPHeader sendSA;
                    srand(currentTimeMs());
                    unsigned long mySeq = (unsigned)(rand()* rand()) % 30721;
                    sendSA.setSeqNumber(mySeq);
                    sendSA.setAckNumber(++clientSeq);
                    sendSA.setFlags(6);
                    sendSA.generate(headerToSend);
                    sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
                    cout << "Sending packet " << sendSA.seqNum() << " 1024 15360 SYN"<<endl;
                    addrlen = sizeof(clientAddr);
                    // trying to receive SYN-ACK
                    TCPHeader recvSA;
                    while(1)
                    {
                        recvlen = recvfrom(sockfd, recvHeader, 8, MSG_PEEK, (struct sockaddr *)&clientAddr, &addrlen);
                        if((clientAddr.sin_port==fclientAddr.sin_port) && (clientAddr.sin_addr.s_addr == fclientAddr.sin_addr.s_addr))
                            recvlen = recvfrom(sockfd, recvHeader, 8, 0, (struct sockaddr *)&clientAddr, &addrlen);
                        else
                            recvlen = -1;
                        if(recvlen == -1)
                        {;}
                        else
                        {
                            recvSA.consume(recvHeader);
                            if(recvSA.flags() == 2)//retransmit SYN-ACK
                            {
                                cout << "Receiving packet " << recvSA.ackNum() << endl;
                                sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
                                cout << "Sending packet " << sendSA.seqNum() << " 15360 1024 Retransmission SYN"<<endl;
                            }
                            else if(recvSA.flags() == 4 && recvSA.seqNum() == clientSeq && recvSA.ackNum() == (++mySeq))
                            {
                                cout << "Receiving packet " << recvSA.ackNum() << endl;
                                break;
                            }
                            else
                            ;//cout<<"Received somthing that is neither SYN nor SYN-ACK!" << endl;
                        }
                    }
                    //check if the numbers correspond
                    if(recvSA.flags() == 4 && recvSA.seqNum() == clientSeq && recvSA.ackNum() == mySeq)
                        {
                            unsigned long cwnd = PKTSIZE;
                            unsigned long ssthresh = 15360;
                            //send file
                            sendFile(fp, filesize, mySeq, recvSA.winSize(), sockfd, clientAddr, cwnd, ssthresh, fclientAddr);
                            //reset the offset
                            fseek(fp,0,SEEK_SET);
                            //close the connection
                            closeConnection(filesize, mySeq, sockfd, clientAddr, cwnd, ssthresh, fclientAddr);
                            // //reset timeout of socket
                            // struct timeval timeout;
                            // timeout.tv_sec = 0;
                            // timeout.tv_usec = 0;
                            // if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                            //  {printf("setsockopt failed\n");return -1;}
                        }
                    else
                        cout << "Handshake failed!" << endl;
                    _exit(0);
                }
            }//fork ends
        }
    }
        else
            ;//cout << "Receiving Error!" << endl;

  }
  /* never exits */
}

void sendFile(FILE* fp, unsigned long filesize, unsigned long myInitialSeq, unsigned short wdSize, int sockfd, sockaddr_in clientAddr, unsigned long& cwnd, unsigned long& ssthresh, sockaddr_in fclientAddr)
{
    cout << endl << "===> Sending the file!" << endl;
    //divide the file into packets
    vector<unsigned> pktSize;//contains all the packets
    int numOfPkt = filesize/PKTSIZE;
    char header[8] = {0};
    TCPHeader HeaderObj;
    HeaderObj.setFlags(0);
    for(int i = 0; i < numOfPkt; i++)
    {
        pktSize.push_back(PKTSIZE+8);
    }

    if(filesize%PKTSIZE != 0)
        {//this is for the last pakcet
            pktSize.push_back((filesize%PKTSIZE)+8);
        }

    //initialize parameters
    queue<int> buffer;//sent but not acked
    int nextToSend = 0;//the number of the packet that is to be sent next
    int numToSend = 0;//number of packets to be sent this round
    bool allSent = false;//control sending, meaning having sent all the packets of the file
    //bool allAcked = false;//control receiving, meaning having received the ack for the last packet
    socklen_t addrlen = sizeof(clientAddr);
    int recvlen = 0;
    unsigned short lastAcked = 0;
    unsigned short newAcked = 0;
    int repeatNum = 0;
    char packet[PKTSIZE+8] = {0};
    map<int, unsigned long> timers;
    bool SS = true;

    //set the socket's time-out
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 25;
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        printf("setsockopt failed\n");

    while(!allSent || !buffer.empty())//repeatedly send and receive
    {
        //send packets of this round
        numToSend = ((wdSize>(unsigned long)cwnd)?(unsigned long)cwnd:wdSize) /PKTSIZE - buffer.size();
        //cout << "numToSend: " << numToSend << endl;
        //cout << "numToSend is: "<<numToSend << " ,cwnd is: "<<cwnd <<endl;
        for(int i = nextToSend; ((i < (nextToSend+numToSend)) && !allSent); i++)
        {
            fillThePkt(packet, pktSize[i]-8, myInitialSeq, i, fp);
            timers[i] = currentTimeMs();//store the time
            sendto(sockfd, packet, pktSize[i], 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
            printf("Sending packet %lu", (i*PKTSIZE+myInitialSeq)%30721);
            cout << " " << cwnd << " " << ssthresh << endl;
            //cout << nextToSend+numToSend << nextToSend <<endl;
            buffer.push(i);//update the buffer
            if(i == (int)pktSize.size()-1)
                {
                    allSent = true;
                    break;
                }
        }
        if(nextToSend < (nextToSend+numToSend))//update
            nextToSend += numToSend;

        //recv
        recvlen = recvfrom(sockfd, header, 8, MSG_PEEK, (struct sockaddr *)&clientAddr, &addrlen);
        if((clientAddr.sin_port==fclientAddr.sin_port) && (clientAddr.sin_addr.s_addr == fclientAddr.sin_addr.s_addr))
            recvlen = recvfrom(sockfd, header, 8, 0, (struct sockaddr *)&clientAddr, &addrlen);
            else
            recvlen = -1;
        if(recvlen != 8)
        {
        if (recvlen != -1)
                {cout << "header length error!" << endl;exit(0);}
        else
                {
                    //cout << recvlen << endl;
                    //cout << "continue!" <<endl;
                    //continue;//recv time out
                }
        }
        else//when recvlen == 8
        {
            HeaderObj.consume(header);
            wdSize = HeaderObj.winSize();
            newAcked = HeaderObj.ackNum();
            printf("Receiving packet %d\n", newAcked);
            if(newAcked == lastAcked)//repeated ack
            {
                repeatNum++;
                if(repeatNum == 3)//repeated 3 times
                    {//start retransmission
                        for(int i = buffer.front(); !buffer.empty() && i <= buffer.back(); i++)
                        {
                            //if this packet is in the buffer
                            if((unsigned long)((unsigned long)i*(unsigned long)PKTSIZE+myInitialSeq)%30721 == newAcked)
                                {//retransmit
                                    ssthresh = cwnd/2;
                                    cwnd = 1024;
                                    SS = true;
                                    fillThePkt(packet, pktSize[i]-8, myInitialSeq, i, fp);
                                    timers[i] = currentTimeMs();//update the timer
                                    sendto(sockfd, packet, pktSize[i], 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
                                    printf("Sending packet %lu ", (i*PKTSIZE+myInitialSeq)%30721);
                                    cout << cwnd << " " << ssthresh << " Retransmission" << endl;

                                }
                        }
                        repeatNum = 0;//reset count
                    }
            }
            else//not repeated ack
            {
                repeatNum = 0;
                lastAcked = newAcked;
            }

            //update the buffer
            for(int i = buffer.front(); !buffer.empty() && (i <= buffer.back()); i++)
            {
                if(((unsigned long)((unsigned long)i*(unsigned long)PKTSIZE+myInitialSeq + (unsigned long)pktSize[i] - 8)%30721) == newAcked)
                    {
                        if(SS)//slow start
                        {cwnd += PKTSIZE;}
                        else//congestion control
                        {cwnd += PKTSIZE*PKTSIZE/cwnd;}
                        if(SS && (cwnd > ssthresh))
                            SS = false;
                        while( (i >= buffer.front()) && !buffer.empty())
                            buffer.pop();
                        break;
                    }
                else
                {;}
            }

        }

        //check the timers
        if(!buffer.empty())
            {
                int n = buffer.front();
                if(timers.find(n) != timers.end())//every packet in the buffer should have corresponding timer
                {
                if(currentTimeMs() - timers[n] >= RTO)
                {//retransmit
                    ssthresh = cwnd/2;
                    cwnd = 1024;
                    SS = true;
                    for(int i = n;i<=buffer.back();i++)//retransmit all the unacked packets
                    {
                        fillThePkt(packet, pktSize[i]-8, myInitialSeq, i, fp);
                        timers[i] = currentTimeMs();//update the timer
                        sendto(sockfd, packet, pktSize[i], 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
                        printf("Sending packet %lu ", (i*PKTSIZE+myInitialSeq)%30721);
                        cout << cwnd << " " << ssthresh << " Retransmission" << endl;
                    }
                }
                }
                else
                {cout << buffer.back() << endl;cout << "packet in buffer but doesn't have corresponding timer!" << endl; exit(0);}
            }

    }
}

void closeConnection(unsigned long filesize, unsigned long myInitialSeq, int sockfd, sockaddr_in clientAddr, unsigned long& cwnd, unsigned long& ssthresh, sockaddr_in fclientAddr)
{
    cout << endl << "===> Closing connection!" << endl;
    socklen_t addrlen = sizeof(clientAddr);
    TCPHeader HeaderObj;
    char headerToSend[8] = {0};
    char receivedHeader[8] = {0};
    HeaderObj.setSeqNumber((unsigned long)(myInitialSeq+filesize));
    HeaderObj.setFlags(1);
    HeaderObj.generate(headerToSend);
    unsigned long time0 = currentTimeMs();
    sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));//send FIN
    cout << "Sending packet " << HeaderObj.seqNum() << " " << cwnd << " " << ssthresh << " FIN"<< endl;
    unsigned short clientSeq = 0;
    int recvlen = 0;

    while(1)//try to receive FIN-ACK
    {
        recvlen = recvfrom(sockfd, receivedHeader, 8, MSG_PEEK, (struct sockaddr *)&clientAddr, &addrlen);
        if((clientAddr.sin_port==fclientAddr.sin_port) && (clientAddr.sin_addr.s_addr == fclientAddr.sin_addr.s_addr))
            recvlen = recvfrom(sockfd, receivedHeader, 8, 0, (struct sockaddr *)&clientAddr, &addrlen);
            else
            recvlen = -1;
        if(recvlen == -1)
            {
                //cerr << "recv FIN ACK error!" << endl;
                ;
            }
        else
            {
                HeaderObj.consume(receivedHeader);
                if(HeaderObj.flags() == 4  && (HeaderObj.ackNum() == (unsigned long)(myInitialSeq+filesize +1)%30721))
                    {cout << "Receiving packet " << HeaderObj.ackNum() << endl;break;}//FIN-ACK received
            }
            //if times out, retransmit FIN
            if(currentTimeMs() - time0 >= RTO)
            {
                time0 = currentTimeMs();//update timer
                sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));//retransmit FIN
                cout << "Sending packet " << HeaderObj.seqNum() << " " << cwnd << " " << ssthresh << " Retransmission FIN"<< endl;
            }
    }

    while(1)
    {
        recvlen = recvfrom(sockfd, receivedHeader, 8, MSG_PEEK, (struct sockaddr *)&clientAddr, &addrlen);
        if((clientAddr.sin_port==fclientAddr.sin_port) && (clientAddr.sin_addr.s_addr == fclientAddr.sin_addr.s_addr))
            recvlen = recvfrom(sockfd, receivedHeader, 8, 0, (struct sockaddr *)&clientAddr, &addrlen);
            else
            recvlen = -1;
        if(recvlen == -1)
            {
                //cerr << "recv FIN ACK error!" << endl;
                ;
            }
        else
            {
                HeaderObj.consume(receivedHeader);
                if(HeaderObj.flags() == 1)
                    {
                        cout << "Receiving packet " << HeaderObj.ackNum() << endl;
                        clientSeq=HeaderObj.seqNum();
                        break;
                    }//FIN received
            }
    }

    //final phase
    HeaderObj.setSeqNumber((unsigned long)(myInitialSeq+filesize+1));
    HeaderObj.setAckNumber(clientSeq+1);
    HeaderObj.setFlags(4);
    HeaderObj.generate(headerToSend);
    unsigned short seq = HeaderObj.seqNum();
    //send FIN-ACK
    cout << "Sending packet " << seq << " " << cwnd << " " << ssthresh << endl;
    time0 = currentTimeMs();
    sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));

    while(1)//wait out the timer
    {
        recvlen = recvfrom(sockfd, receivedHeader, 8, MSG_PEEK, (struct sockaddr *)&clientAddr, &addrlen);
        if((clientAddr.sin_port==fclientAddr.sin_port) && (clientAddr.sin_addr.s_addr == fclientAddr.sin_addr.s_addr))
            recvlen = recvfrom(sockfd, receivedHeader, 8, 0, (struct sockaddr *)&clientAddr, &addrlen);
            else
            recvlen = -1;
        if(recvlen == -1)
            {
                //cerr << "recv FIN ACK error!" << endl;
                ;
            }
        else
            {
                HeaderObj.consume(receivedHeader);
                if(HeaderObj.flags() == 1)
                    {
                        cout << "Receiving packet " << HeaderObj.ackNum() << endl;
                        cout << "Sending packet " << seq << " " << cwnd << " " << ssthresh << " Retransmission" <<endl;
                        time0 = currentTimeMs();
                        sendto(sockfd, headerToSend, 8, 0, (struct sockaddr *)&fclientAddr, sizeof(fclientAddr));
                    }
            }
        if(currentTimeMs()-time0 >= 2*RTO)
            break;//timer ends, connection cloesd
    }
    cout << "Connection closed!" << endl;

}
