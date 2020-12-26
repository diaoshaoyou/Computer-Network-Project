#pragma once
#pragma once
#ifndef MY_PACKET_H
#define MY_PACKET_H

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "global.h"
#pragma comment(lib,"ws2_32.lib")


//The event of the main thread
HANDLE InputEvent = NULL;
//The event of the child thread
HANDLE ReceiveEvent = NULL;
//The status whether the connection is established or not
bool ConnectStatus = false;
//The clinet's socket
SOCKET ClientSocket = 0;
//The buffer to receive the response of server
char ReceiveBuf[512] = { 0 };
//The buffer to be sented to the server
char RequestBuf[512] = { 0 };
//The IP address of server
char ServerIP[100] = { 0 };


//Initialize the socket
void InitSocket(void);
//Create the child thread
void create_child_thread(void);
//The function to be called by the main thread
DWORD WINAPI mainThrFun(LPVOID lp);
//The function to be called by the child thread
DWORD WINAPI childThrFun(LPVOID lp);

//Establish connection between the client and the server
void connect_to_server();
//Exit the program
void exit();
//Break connection between the client and the server
void disconnect();
//Send request to server for current time
void send_time_request();
//Send request to server for server's name
void send_name_request();
//Send request to server for all clients' list
void send_list_request();
//Send message to another client
void send_message_request();

#endif