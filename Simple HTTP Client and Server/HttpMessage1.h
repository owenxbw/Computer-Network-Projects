/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/

/*
* File:   HttpMessage.h
* Author: MIGYEONG
*
* Created on October 22, 2016, 4:43 PM
*/

#ifndef HTTPMESSAGE_H
#define HTTPMESSAGE_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

using namespace std;

/*
* Data Abstraction Example from
* https://www.tutorialspoint.com/cplusplus/cpp_data_abstraction.htm
* Inheritance Example from
* https://www.tutorialcup.com/cplusplus/inheritance.htm
*
* Base Class: HttpMessage
*/
class HttpMessage {
public:
	string Url;   // same as file directory: "/index.html"
	string Method;   // "GET"
	string Version;   // "HTTP/1.0" or "HTTP/1.1"
	string::size_type n;
	string::size_type n_tmp;

	// After parsing url
	string serverName;   // same as host name: "localhost" or "www-net.cs.umass.edu"
	int portNumber;    // "4000"
	string filename;   // "index.html"
	string connectionStatus;   // "close" or "keep-alive"

	int statusCode;   // "200" means file exist, "404" means file doesn't exist, "400" bad request format
	int ContentLength;   // size of the file
	string fileContent;

	HttpMessage() {
		Url = "";
		Method = "";
		Version = "";
		serverName = "";
		filename = "";
		connectionStatus = "";
		statusCode = 0;
		ContentLength = 0;
		fileContent = "";
		portNumber = 80;
	}

	int string_to_int(string s)
	{
		istringstream convert(s);
		int n;
		convert >> n;
		return n;
	}

	void setUrl(string input) {

		// Parsing Example from
		//http://stackoverflow.com/questions/1011339/how-do-you-make-a-http-request-with-c

		// npos is a static member constant value with the greatest
		// possible value for an element of type size_t.

		// rfind() Searches the string for the last occurrence of the sequence
		// specified by its arguments.

		string copyURL = input;

		if (copyURL.substr(0, 7) == "http://") {
			copyURL.erase(0, 7);
		}

		if (copyURL.substr(0, 8) == "https://") {
			copyURL.erase(0, 8);
		}

		n = copyURL.find('/');
		if (n != string::npos) {
			string tmp = copyURL.substr(0, n);
			n_tmp = tmp.find(":");
			if (n_tmp != string::npos) {
				serverName = tmp.substr(0, n_tmp);
				portNumber = string_to_int(tmp.substr(n_tmp + 1, n));
			}
			else {
				serverName = tmp;
			}

			Url = copyURL.substr(n);   // same as the path of a file
			n = Url.rfind('/');
			filename = Url.substr(n + 1);
		}
		else {
			serverName = copyURL;
			Url = "/";     // same as the path of a file
			filename = "";
		}
	}

	void setMethod(string input) {
		Method = "GET";
	}

	void setVersion(string input) {
		Version = "HTTP/1.1";
	}

	// Get a response message
	string getResponseCode(int code) {
		string Message = "";
		switch (code) {
		case 200:
			Message = "200 OK";
			break;
		case 400:
			Message = "400 Bad request";
			break;
		case 404:
			Message = "404 Not found";
			break;
		case 403:
			Message = "403 Forbidden";
			break;
		case 501:
			Message = "501 Not implemented";
			break;
		case 505:
			Message = "505 HTTP version not supported";
			break;
		default:
			Message = "Cannot define HTTP response status codes!";
			break;
		}
		return Message;
	}

	string setConnection(int i) {

		switch (i) {
		case 1:
			// No more transactions will proceed.
			// The connection closes after the current one.
			connectionStatus = "close";
			break;
		case 2:
			connectionStatus = "keep-alive";
			break;
		default:
			break;
		}
		return connectionStatus;
	}

	// Encode the message into a string of bytes of the wire format
	vector<uint8_t> encode() {
		// Conversion Example from
		// http://stackoverflow.com/questions/7664529/converting-a-string-to-uint8-t-array-in-c

		string GETrequest = Method + " " + Url + " " + Version + "\r\n\r\n";

		vector<uint8_t> bytes(GETrequest.begin(), GETrequest.end());
		bytes.push_back('\0');
		uint8_t *p = &bytes[0];
		return bytes;
	}

	// Decode the message
	string consume(vector<uint8_t> wire) {
		string str(wire.begin(), wire.end());
		return str;
	}
};

/*
* Derived Class 1: HttpRequest (for HTTP message abstraction)
*/
class HttpRequest : public HttpMessage {
public:
	// Constructor
	HttpRequest() {

	}

	string setReqMessage() {
		string msg = Method + " " + Url + " " + Version + "\r\n" +
			"Host: " + serverName + "\r\n" +
			"Connection: " + setConnection(2) + "\r\n\r\n";
		return msg;
	}

	void getReqMessage(string msg) {
		string copyMsg = msg;

		// Get Method
		if (copyMsg.substr(0, 3) == "GET") {
			Method = copyMsg.substr(0, 3);
			copyMsg.erase(0, 4);
		}
		else {
			// 400 Bad request!
			statusCode = 400;
		}

		// Get Url
		n = copyMsg.find("HTTP");
		if (n != string::npos) {
			Url = copyMsg.substr(0, n - 1);   //same as the path of a file
			filename = copyMsg.substr(1, n - 1);
			copyMsg.erase(0, n);

		}
		else {
			// 400 Bad request!
			statusCode = 400;
		}

		// Get Version
		n = copyMsg.find("\r\n");
		if (n != string::npos) {
			Version = copyMsg.substr(0, n);
			copyMsg.erase(0, n + 2);
		}
		else {
			// 400 Bad request!
			statusCode = 400;
		}

		// Get Server name from "Host: " header line
		n = copyMsg.find("Host: ");
		copyMsg.erase(0, n + 6);

		n = copyMsg.find("\r\n");
		if (n != string::npos) {
			serverName = copyMsg.substr(0, n);   //same as the host name
			copyMsg.erase(0, n);
		}
		else {
			// 400 Bad request!
			statusCode = 400;
		}

		//        return "Method - " + Method + "\n" +
		//                "Url - " + Url + "\n" +
		//                "FileName - " + filename + "\n" +
		//                "Version - " + Version + "\n" +
		//                "ServerName - " + serverName + "\n";
	}
};

/*
* Derived Class 2: HttpResponse (for HTTP message abstraction)
*/
class HttpResponse : public HttpMessage {
public:
	// Constructor
	HttpResponse() {

	}

	// Create HTTP Response message
	string setResMessage() {
		string msg = "HTTP/1.1 " + getResponseCode(statusCode) + "\r\n" +
			"Content-Length: " + to_string(ContentLength) + "\r\n" +
			"Connection: " + setConnection(2) + "\r\n\r\n";
		return msg;
	}

	void getResMessage(string msg) {
		string copyMsg = msg;
		// Find Version
		Version = copyMsg.substr(0, 8);
		copyMsg.erase(0, 9);   // including the space

							   // Find Status Code
		n = copyMsg.find("\r\n");
		if (n != string::npos) {
			if (copyMsg.substr(0, n) == "200 OK") {
				statusCode = 200;
			}
			else if (copyMsg.substr(0, n) == "404 Not found") {
				statusCode = 404;
			}
			else if (copyMsg.substr(0, n) == "400 Bad request") {
				statusCode = 400;
			}
			else if (copyMsg.substr(0, n) == "505 HTTP version not supported") {
				statusCode = 505;
			}
			else {
				statusCode = 0;
			}
			copyMsg.erase(0, n + 2);
		}

		// Find Content-Length
		n = copyMsg.find("Content-Length: ");
		if (n != string::npos) {
			copyMsg.erase(0, n + 16);

			n = copyMsg.find("\r\n");
			string cl = copyMsg.substr(0, n);
			ContentLength = string_to_int(cl);
			copyMsg.erase(0, n + 2);
		}
	}

	string getFileContent(string msg) {
		// Find the file content
		n = msg.find("\r\n\r\n");
		if (n != string::npos) {
			fileContent = msg.substr(n + 4);
		}
		return fileContent;
	}

	string showResMessage() {
		string resmsg = Version + " " + getResponseCode(statusCode) + "\r\n" +
			"Content-Length: " + to_string(ContentLength) + "\r\n" +
			"Connection: " + setConnection(2) + "\r\n\r\n";

		return resmsg;
	}

};

#endif /* HTTPMESSAGE_H */