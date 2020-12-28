#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <streambuf> 
#include <fstream>
#include <string>
#include <thread>
#include <vector>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 10000
#define DEFAULT_PORT "5426"

int func(SOCKET ClientSocket)
{
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN] = "\0";
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		printf("Bytes received: %d\n", iResult);
		printf("received content: %s", recvbuf);

		//first check out whether the packet have double '\n'
		int pos = 0;
		while (pos < iResult)//check the whole packet
		{
			if (recvbuf[pos] == '\r' && recvbuf[pos + 1] == '\n' && pos < iResult - 3 && recvbuf[pos + 2] == '\r' && recvbuf[pos + 3] == '\n')
				break;
			pos++;
		}
		if (pos == iResult)
		{
			printf("the packet is wrong!\n");
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		//then deal with the content: 1.Get the necessary information
		std::string receivePacket = recvbuf;
		pos = receivePacket.find(" ");
		std::string method = receivePacket.substr(0, pos);
		pos++;
		std::string relativePath = receivePacket.substr(pos, receivePacket.find(" ", pos) - pos);
		std::string absolutePath;
		if (relativePath.compare("/favicon.ico") == 0)
			absolutePath = "D:/GitHub/Computer-Network-lab8/lab8/html/favicon.ico";
		else
			absolutePath = "D:/GitHub/Computer-Network-lab8/lab8/html/" + relativePath.substr(5);
		std::string fileType = relativePath.substr(relativePath.find(".") + 1);
		std::string sendPacket = "";
		char sendbuf[1000000];
		ZeroMemory(sendbuf, 1000000);

		if (method.compare("GET") == 0)
		{
			std::ifstream fin(absolutePath, std::ifstream::binary);
			char* content = NULL;
			int contentLength = 0;

			bool isExist = true;
			if (!fin)//the file doesn't exist
				isExist = false;
			else
			{
				fin.seekg(0, fin.end);
				contentLength = fin.tellg();
				fin.seekg(0, fin.beg);
				content = new char[contentLength];
				fin.read(content, contentLength);
				if (fin)
					std::cout << "all characters read successfully.";
				else
					std::cout << "error: only " << fin.gcount() << " could be read";
				fin.close();
			}

			sendPacket += "HTTP/1.1 ";
			if (isExist)
				sendPacket += "200 OK\r\n";
			else
				sendPacket += "404 Not Found\r\n";
			if (fileType.compare("html") == 0)
				sendPacket += "Content-Type: text/html\r\n";
			else if (fileType.compare("jpg") == 0)
				sendPacket += "Content-Type: application/x-jpg\r\n";
			else if (fileType.compare("txt") == 0)
				sendPacket += "Content-Type: text/plain\r\n";

			sendPacket += "Content-Length: ";
			if (isExist)
				sendPacket += std::to_string(contentLength);
			else
				sendPacket += std::to_string(0);
			sendPacket += "\r\n\r\n";
			if (isExist)
			{
				strcpy(sendbuf, sendPacket.c_str());
				memcpy(&sendbuf[sendPacket.size()], content, contentLength);
				delete[] content;
			}
			else
			{
				strcpy(sendbuf, sendPacket.c_str());
				delete[] content;
			}

		}
		else if (method.compare("POST") == 0)
		{
			bool isPOST = true;
			pos = receivePacket.find("Content-Length: ") + 16;
			int contentLength = stoi(receivePacket.substr(pos, receivePacket.find(" ", pos) - pos));
			if (relativePath.compare("/dir/dopost") != 0)
				isPOST = false;
			pos = 0;
			while (pos < iResult)//check the whole packet
			{
				if (recvbuf[pos] == '\r' && recvbuf[pos + 1] == '\n' && pos < iResult - 3 && recvbuf[pos + 2] == '\r' && recvbuf[pos + 3] == '\n')
					break;
				pos++;
			}
			pos += 4;
			std::string body = receivePacket.substr(pos, contentLength);
			bool isSuccess = true;
			if (body.find("login=3150105426&") == std::string::npos)
				isSuccess = false;
			pos = body.find("=5426");
			if (body.find("pass=5426") == std::string::npos || (pos + 6) <= body.size())
				isSuccess = false;
			sendPacket += "HTTP/1.1 ";
			if (isPOST)
				sendPacket += "200 OK\r\n";
			else
				sendPacket += "404 Not Found\r\n";
			sendPacket += "Content-Type: text/html\r\n";
			std::string successDispaly = "<html><body>Success!</body></html>";
			std::string failedDisplay = "<html><body>Not Success!</body></html>";
			sendPacket += "Content-Length: ";
			if (isPOST)
			{
				if (isSuccess)
					sendPacket += std::to_string(successDispaly.size());
				else
					sendPacket += std::to_string(failedDisplay.size());
			}
			else
				sendPacket += std::to_string(0);
			sendPacket += "\r\n\r\n";

			if (isPOST)
			{
				if (isSuccess)
					sendPacket += successDispaly;
				else
					sendPacket += failedDisplay;
				strcpy(sendbuf, sendPacket.c_str());
			}
			else
			{
				strcpy(sendbuf, sendPacket.c_str());
			}
		}

		// Echo the buffer back to the sender
		iSendResult = send(ClientSocket, sendbuf, 1000000, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		printf("------------------------------------\n");
		printf("Bytes sent: %d\n", iSendResult);
		printf("send content: %s", sendPacket.c_str());
	}
	else if (iResult == 0)
		printf("Connection closing...\n");
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	iResult = shutdown(ClientSocket, SD_SEND);
	closesocket(ClientSocket);
	return 0;
}

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	do {
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Accept a client socket
		SOCKET ClientSocket = INVALID_SOCKET;
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		std::thread(func, std::move(ClientSocket)).detach();

	} while (true);

	// No longer need server socket
	closesocket(ListenSocket);
	// shutdown the connection since we're done
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// cleanup
	WSACleanup();

	return 0;
}