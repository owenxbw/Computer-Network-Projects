/* C functionalities */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>

/* C networking functionalities */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* C++ functionalities */
#include <string>
#include <iostream>
#include <sstream>

#include "HttpMessage1.h"

using namespace std;
#define BUFSIZE 30

void startconnection(int& sockfd, string& direc);

int
main(int argc, char* argv[])
{
	//check the inputs
	if (argc == 1)
	{
		argv[1] = "localhost";
		argv[2] = "4000";
		argv[3] = ".";
	}
	else if (argc != 4)
	{
		perror("3 arguments required!");
		return 1;
	}

	//check the port number
	for (int i = 0; (argv[2])[i] != '\0'; i++)
	{
		if ((argv[2])[i] <48 || (argv[2])[i] > 57)
		{
			perror("port number needs to be an integer!");
			return 2;
		}
	}
	//check the directory
	string direc = argv[3];
	if (direc != ".")
		direc = '.' + direc;
	if (int dir = open(direc.c_str(), O_RDONLY) == -1)
	{
		perror("wrong directory!"); return 3;
	}
	else
		close(dir);

	//show ip
	struct addrinfo hints;
	struct addrinfo* res;
	// prepare hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
									 // get address
	int status = 0;
	if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
	{
		std::cerr << "wrong hostname: " << gai_strerror(status) << std::endl;
		return 1;
	}

	// convert address to IPv4 address
	struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;

	// convert the IP to a cstring
	char hostipstr[INET_ADDRSTRLEN] = { '\0' };
	inet_ntop(res->ai_family, &(ipv4->sin_addr), hostipstr, sizeof(hostipstr));
	freeaddrinfo(res); // free the linked list

					   // create a socket using TCP IP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// allow others to reuse the address
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		return 2;
	}

	// bind address to socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));     // short, network byte order
	addr.sin_addr.s_addr = inet_addr(hostipstr);
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		return 2;
	}
	// set socket to listen status
	if (listen(sockfd, 1) == -1)
	{
		perror("listen");
		return 3;
	}
	//recursively starts a new connection for multiprocess
	startconnection(sockfd, direc);
}


void startconnection(int& sockfd, string& direc)
{
	// accept a new connection
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);

	int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
	if (clientSockfd == -1)
	{
		perror("accept");
		return;
	}
	//child process starts here
	if (!fork())
	{
		char ipstr[INET_ADDRSTRLEN] = { '\0' };
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

		//send/receive data to/from connection
		bool EndOfMsg = false;
		char buf[BUFSIZE] = { 0 };
		string requestmsg;

		while (1) {

			while (!EndOfMsg)
			{
				memset(buf, '\0', sizeof(buf));
				int size = read(clientSockfd, buf, sizeof(buf));
				if (size == -1)
					perror("cannot read!");
				else
					requestmsg += buf;
				//check if the msg is completed
				if (requestmsg.find("\r\n\r\n") == (requestmsg.size() - 4))
					EndOfMsg = 1;
			}
			EndOfMsg = 0;

			cout << "requestMessage is:" << endl << requestmsg;
			cout << "requestSize is " << requestmsg.size() << endl;
			//construct the HttpRequest
			HttpRequest request;
			request.getReqMessage(requestmsg);
			requestmsg = "";
			//Bad Request
			if (request.statusCode == 400)
			{
				cerr << "Bad Request!" << endl;
				HttpResponse response;
				response.statusCode = 400;
				string resstr = response.setResMessage();
				if (send(clientSockfd, resstr.c_str(), resstr.size(), 0) == -1)
					cerr << "cannot send!1" << endl;
				continue;
			}
			//check the request
			if (request.Method != "GET")
			{
				cerr << "Only expecting GET!" << endl;
				HttpResponse response;
				response.statusCode = 400;
				string resstr = response.setResMessage();
				if (send(clientSockfd, resstr.c_str(), resstr.size(), 0) == -1)
					cerr << "cannot send!1" << endl;
				continue;
			}
			//check the host name
			if ((request.serverName.find("localhost") == string::npos) && (request.serverName.find("Localhost") == string::npos) && (request.serverName.find("LOCALHOST") == string::npos))
			{
				cerr << "Wrong host name!" << endl;
				HttpResponse response;
				response.statusCode = 400;
				string resstr = response.setResMessage();
				if (send(clientSockfd, resstr.c_str(), resstr.size(), 0) == -1)
					cerr << "cannot send!2" << endl;
				continue;
			}
			cout << "hostname is: " << request.serverName << endl;
			if (request.Version != "HTTP/1.1")
			{
				cerr << "Only expecting HTTP/1.1!" << endl;
				HttpResponse response;
				response.statusCode = 505;
				string resstr = response.setResMessage();
				if (send(clientSockfd, resstr.c_str(), resstr.size(), 0) == -1)
					cerr << "cannot send!1" << endl;
				continue;
			}

			//construct the HttpResponse
			HttpResponse response;
			cout << direc + request.Url << " requested" << endl;

			//check if the file exists
			int fd = open((direc + request.Url).c_str(), O_RDONLY);
			if (fd == -1 || request.Url.substr(request.Url.size() - 1, 1) == "/")
			{
				cout << "File doesn't exist!" << endl;

				//construct the response
				response.statusCode = 404;
				string resstr = response.setResMessage();
				if (send(clientSockfd, resstr.c_str(), resstr.size(), 0) == -1)
					perror("cannot send!1");
			}
			else
			{
				cout << "file exits!" << endl;
				cout << "Encoding.." << endl;
				//construct the response
				response.statusCode = 200;
				//encode all the information
				//get the filesize
				struct stat st;
				fstat(fd, &st);
				int filesize = st.st_size;
				cout << "filesize is " << filesize << endl;
				int size = 0;
				response.ContentLength = filesize;
				string resstr = response.setResMessage();
				cout << "Sending..." << endl;

				if ((size = send(clientSockfd, resstr.c_str(), resstr.size(), 0)) == -1)
					perror("cannot send!5");
				close(fd);
				//transfer the file
				FILE * fp = fopen((direc + request.Url).c_str(), "r");
				if (NULL == fp)
				{
					printf("File:\t%s Not Found\n", (direc + request.Url).c_str());
				}
				else
				{
					memset(buf, '\0', BUFSIZE);
					int size = 0;
					while ((size = fread(buf, sizeof(char), BUFSIZE, fp))>0)
					{
						if ((size = send(clientSockfd, buf, size, 0))<0)
						{
							printf("Send File:\t%s Failed\n", (direc + request.Url).c_str());
							break;
						}
						memset(buf, '\0', BUFSIZE);
					}
					fclose(fp);
				}
				cout << "done!" << endl;
			}

		}
	}
	close(clientSockfd);
	startconnection(sockfd, direc);
}