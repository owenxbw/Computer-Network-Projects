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
#include "HttpMessage1.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

int
main(int argc, char* argv[])
{
	int inputNum;
	inputNum = argc;

	string input;
	string reqMsg[inputNum];
	int port_num[inputNum];
	string hostname[inputNum];
	string filename[inputNum];

	// check the inputs
	if (argc == 1) {
		perror("URL required!");
		return 1;
	}

	// create a socket using TCP IP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);  //int socket(int family, int type, int protocol);

	for (int i = 1; i < inputNum; ++i)
	{
		input = argv[i];

		//call from HttpRequest
		HttpRequest request;
		request.setUrl(input);
		request.setMethod(input);
		request.setVersion(input);

		// Set up HTTP Request
		reqMsg[i] = request.setReqMessage();
		port_num[i] = request.portNumber;
		hostname[i] = request.serverName;
		filename[i] = request.filename;

		if (request.filename.size() == 0) {
			cout << "Please input filename" << endl;
			break;
		}
	}

	// check port_num & hostname are all same on the array
	for (int i = 2; i < inputNum; ++i) {
		if (port_num[i] != port_num[i - 1]) {
			cout << "Have different Port Number on URLs" << endl;
			cout << "Cannot Connect!" << endl;
			return 1;
		}

		if (hostname[i] != hostname[i - 1]) {
			cout << "Have different Host Name on URLs" << endl;
			cout << "Cannot Connect!" << endl;
			return 1;
		}
	}

	/*
	* Start Connection
	*/

	//Find IP address by hostname
	struct addrinfo hints;
	struct addrinfo* res;

	// prepare hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP

	// get IP address and store it to hostname
	int status = 0;
	if ((status = getaddrinfo(hostname[1].c_str(), "80", &hints, &res)) != 0) {
		//report invalid hostname
		cerr << "getaddrinfo: " << gai_strerror(status) << endl;
		cout << "Please input a valid hostname" << endl;
		return 2;
	}
	for (struct addrinfo* p = res; p != 0; p = p->ai_next) {
		// convert address to IPv4 address
		struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

		// convert the IP to a string and print it and save to hostname:
		char ipstr[INET_ADDRSTRLEN] = { '\0' };
		inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
		hostname[1] = ipstr;}
	freeaddrinfo(res); // free the linked list


	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port_num[1]);     // short, network byte order
	serverAddr.sin_addr.s_addr = inet_addr(hostname[1].c_str());
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	// connect to the server
	if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("connect");
		return 2;
	}


	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
		perror("getsockname");
		return 3;
	}

	char ipstr[INET_ADDRSTRLEN] = { '\0' };
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));

	// Send/Receive all HTTP Request/Response
	for (int i = 1; i < inputNum; ++i) {
		if (send(sockfd, reqMsg[i].c_str(), reqMsg[i].size(), 0) == -1) {
			perror("send");
			return 4;
		}

		int buf_size = 30;
		char buf[30] = { 0 };

		string delimiter = "\r\n\r\n";
		string data;
		string HTTPHeader;
		string responsemsg;
		string bufstr;

		HttpResponse response;

		//create file
		int size = 0;
		string::size_type pos = 0;                             //guarantee to have enough size
		while ((size = recv(sockfd, buf, buf_size, 0)) > -1)   //**
		{
			responsemsg += bufstr.assign(buf, size);
			pos = responsemsg.find(delimiter);
			if (pos != string::npos){                                                //string::npos can be substituted by -1?
				response.getResMessage(responsemsg.substr(0, (pos + 4)));
				break;
			}
			memset(buf, '\0', buf_size);
		}

		int count = response.ContentLength - responsemsg.size() + pos + 4;

		while (count > 0)
		{
			if (count > buf_size)
			{
				if ((size = recv(sockfd, buf, buf_size, 0)) == -1)
					cerr << "cannot read!" << endl;
				else
					responsemsg += bufstr.assign(buf, size);
				count -= size;
			}
			else
			{
				if ((size = recv(sockfd, buf, count, 0)) == -1)
					cerr << "cannot read!" << endl;
				else
					responsemsg += bufstr.assign(buf, size);
				count = count - size;
			}
			memset(buf, '\0', buf_size);

		}

		cout << responsemsg.substr(0, pos + 4) << endl;
		//cout << responsemsg.substr(pos+4).size() <<endl;

		//create file
		if (response.statusCode == 200) {
			std::ofstream outfile(filename[i]);
			outfile << responsemsg.substr(pos + 4);
		}

	}

	close(sockfd);

}