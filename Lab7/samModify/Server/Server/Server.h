#pragma once
#ifndef MY_PACKET_H
#define MY_PACKET_H

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include "global.h"
#pragma comment(lib,"ws2_32.lib")

//The header,s length
#define HEADERLENTH 2
//The max number of clients
#define MAXQUEUE 100

//The struct to record a client's information
typedef struct my_client {
	//The client's address
	SOCKADDR_IN SocAddress;
	//The client's socket
	SOCKET CurrentSocket;
	//The flag to indicate wheather the client is connected or not
	int DirtyBit;
	HANDLE ChildThread;
}MyClient;

//The struct to record the child thread and corresponding index
typedef struct child
{
	int index;
	SOCKET ChildSocket;

}ChildClient;

//The list of all clients
MyClient ClientList[MAXQUEUE];
//Initialize the socket
void InitSocket(void);
//Create the main thread
void CreateMainThr(void);
//Create the child thread
HANDLE createChildThr(ChildClient* ChildPtr);
//The function to be called by the child thread
DWORD WINAPI childThrFun(LPVOID lp);
//The server's socket
SOCKET ServerSocket = 0;
//The client's socket
SOCKET ClientSocket = 0;
//The IP address
SOCKADDR_IN addr;
//The length of IP address
int addrlen = sizeof(SOCKADDR);
//The temp buffer
char Buffer[MAXDATALENTH];

//The bind function
void BindSocket(void);
//The listen function
void Listen(void);
//The function to send message
void send_message(char* request, char* reply);
//The function to response the clients' list
void pack_list(char* buffer);
//The function to response the server's name
void pack_name(char* buffer);
//The function to response the current time
void pack_time(char* buffer);


#endif