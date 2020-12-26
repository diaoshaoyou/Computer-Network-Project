#pragma once
#define _CRT_SECURE_NO_WARNINGS
//头文件
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include "global.h"

//加入动态连接库
#pragma comment(lib, "ws2_32.lib")

//宏
#define _HEADER_LEN 2
#define _MAX_DATA_LEN 128
#define _MAX_Q_SIZE 500

//定义结构
//typedef enum {//this is the package that the server might receive
//	NAME,
//	LIST,
//	TIME,
//	DATA,
//	DISCONN,
//	CONN, //TO ZHOU：我觉得应该在这里加上几个控制信号，client要告诉服务器需要连接和退出
//	EXIT 
//}ReqOp;
typedef struct client_Request { //a request sended by client
	SOCKADDR_IN soc_address;
	SOCKET curr_socket;
	int dirty_bit;
	HANDLE child_thrd;
}cl_req;
typedef struct child_request { //the request handled by server
	int seq_n;//sequence number
	SOCKET child_sock; //the socket that the client resident
}child_req;

//定义全局变量
int server_port = _DEFAULT_PORT;
cl_req req_list[_MAX_Q_SIZE];
SOCKET server_sock = 0;
SOCKET client_sock = 0;
SOCKADDR_IN addr;
int addrlen = sizeof(SOCKADDR);
char Buffer[_MAX_DATA_LEN];

//函数原型,大写字母开头的是开放的借口，小写的是内部使用的
void Ini_Socket(void);
void Bind(void);
void Listen(void);
void Create_main_thrd(void);

HANDLE create_child_thrd(child_req* child_req_ptr);
DWORD WINAPI child_thrd_func(LPVOID lp);
void send_data(char* se_req, char* se_reply);
void pack_list(char* buffer);
void pack_name(char* buffer);
void pack_time(char* buffer);

//函数实现
void Ini_Socket(void) {
	WSADATA wsa_data;
	WORD wVersianRequested = MAKEWORD(2, 2); //使用的WinSock_DLL版本

	//make sure winsock dll exist.
	if (WSAStartup(wVersianRequested, &wsa_data) != 0) {
		printf("wsa start up failed, check your winsock installation...\n");
		return;
	}

	//make sure winsock dll is at least 2.2
	if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
		WSACleanup();
		printf("Winsock Version should be 2.2, check your winsock version");
		return;
	}

	//init dirty_bit
	for (int i = 0; i < _MAX_Q_SIZE; i++) 
		req_list[i].dirty_bit = 0;
	
	//show back
	printf("Initializing success\n");
	return;
}
void Create_main_thrd(void) {
	printf("Main thread ID: %4d create\n", GetCurrentThreadId());
	printf("--------------------------\n");
	printf("  Welcome to Socket Server \n");
	printf("---------------------------\n");
	
	int i = 0;
	child_req cur_req;
	while (1) {
		printf("listen to port 3574......\n");

		//accept a request from client, if any.
		client_sock = accept(server_sock, (SOCKADDR*)(&addr), &addrlen);
		if (client_sock == INVALID_SOCKET){
			printf("accept error!");
			break;
		}
		i = 0;
		while (++i < _MAX_Q_SIZE) {

			//find a proper location in the maintained request list
			if (req_list[i].dirty_bit == 0) {
				cur_req.child_sock = client_sock;
				cur_req.seq_n = i;

				req_list[i].curr_socket = client_sock;
				req_list[i].dirty_bit = 1;
				req_list[i].soc_address = addr;
				req_list[i].child_thrd = create_child_thrd(&cur_req);
			}
		}
		//break if too many request
		if (i == _MAX_Q_SIZE) {
			printf("reach the maximum of queue, to many requests......\n");
			break;
		}	
	}
	closesocket(server_sock);
	printf("exit main thread\n");
	WSACleanup();
	return;
}
void Bind(void) {
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock == INVALID_SOCKET) {
		WSACleanup();
		printf("get socket fail, check local socket\n");
		return;//
	}
	int len = sizeof(SOCKADDR);
	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(_DEFAULT_PORT);
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(server_sock, (SOCKADDR*)&server, len) == INVALID_SOCKET) {
		printf("bind server failed\n");
		closesocket(server_sock);
		WSACleanup();
		return;
	}
	printf("Bind successfully\n");
	return;
}
void Listen(void) {
	if (listen(server_sock, _MAX_Q_SIZE) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(server_sock);
		WSACleanup();
		return;
	}
	printf("listen success\n");
}

HANDLE create_child_thrd(child_req* child_req_ptr){
	/* this funct is implemented to handle a child request sent by a single
	 * client, used in funct Create_main_thrd
	 */
	/* mainly use win32API CreateThrd,
	 * and call funct child_thrd_func to implmt
	 */
	HANDLE child_thrd = NULL;
	DWORD thrdID = 0;
	child_thrd = CreateThread(
								NULL, 
								0,
				
				child_thrd_func, 
								(LPVOID)child_req_ptr, 
								0, 
								&thrdID);
	if (child_thrd == NULL || thrdID == 0) {
		printf("create child thread failed. maybe lossing all memory\n");
		return NULL;
	}
	return child_thrd;
}
DWORD WINAPI child_thrd_func(LPVOID lp) {
	/* This is the funct that handle the request of the single client currently handled by the server
	 * It is done in switch-case style according the opcode request by client
	 * Used in funct create_child_thrd.
	 */
	printf("child thread ID %4d create\n", GetCurrentThreadId());

	char req_buffer[_MAX_DATA_LEN]; //used to buffer the data that received from client
	memset(req_buffer, 0, sizeof(req_buffer));

	char resp_buffer[_MAX_DATA_LEN]; //used to buffer the data that will be send to client 
	memset(req_buffer, 0, sizeof(req_buffer));

	char seqTmp[10];//used to buffer the sequence number 
	memset(seqTmp, 0, sizeof(seqTmp));

	//first send client a information;
	//to check that the link is OK;
	child_req* ptr = (child_req*)lp;
	SOCKET cur_sock = ptr->child_sock;
	int cur_seq = ptr->seq_n;
	sprintf(seqTmp, "%d", cur_seq);
	strcat(resp_buffer + 2, "Hello");
	strcat(resp_buffer + 2, seqTmp);
	strcat(resp_buffer + 2, "Connect successfully");

	int ret = send(cur_sock, resp_buffer, sizeof(resp_buffer), 0);
	if (ret <= 0)
		printf("3-way handshake failed\n");

	while (1){
		memset(resp_buffer, 0, sizeof(resp_buffer));
		memset(req_buffer, 0, sizeof(req_buffer));
		
		//receive the request client gives
		ret = recv(cur_sock, req_buffer, sizeof(req_buffer), 0);
		if (ret <= 0) {
			printf("recv failed");
			break;
		}

		//according the opcode requested by the client, finish the task
		int op = req_buffer[0];
		if (op == DISCONN){
			req_list[cur_seq].dirty_bit = 0;
			printf("client %d log out\n", cur_seq);
			closesocket(cur_sock);
			break;
		}
		switch (op){
		case TIME:
			pack_time(resp_buffer); //self_defined funct send the name
			break;
		case NAME:
			pack_name(resp_buffer); //like above
			break;
		case LIST:
			pack_list(resp_buffer);
			break;
		case DATA:
			printf("send data\n");
			send_data(req_buffer, resp_buffer);
			break;
		default:
			printf("unrecognized opcode?\n");
			break;
		}

		//send the packed msg
		printf("pack data done, prepare to send to client %d:%s\n", cur_seq, resp_buffer + 2);
		ret = send(cur_seq, resp_buffer, sizeof(resp_buffer), 0);
		if (ret <= 0)
			printf("send failed after pack done, please check connect\n");

		printf("send done.");
	}
	return 0;
}
void pack_time(char* buffer) {
	/* pack the time to the buffer string,
	 * used in funct child_thrd_func
	 */

	time_t t;
	struct tm* t_in_use;
	time(&t);
	t_in_use = localtime(&t);

	char time_tmp[_MAX_DATA_LEN];
	memset(time_tmp, 0, sizeof(time_tmp));

buffer[0] = TIME;
buffer[1] = 1;
sprintf(time_tmp, "%d", t_in_use->tm_year + 1900);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, "/");
sprintf(time_tmp, "%d", t_in_use->tm_mon + 1);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, "/");
sprintf(time_tmp, "%d", t_in_use->tm_mday);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, " ");
sprintf(time_tmp, "%d", t_in_use->tm_hour);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, ":");
sprintf(time_tmp, "%d", t_in_use->tm_min);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, ":");
sprintf(time_tmp, "%d", t_in_use->tm_sec);
strcat(buffer + _HEADER_LEN, time_tmp);
strcat(buffer + _HEADER_LEN, "\0");
}
void pack_name(char* buffer) {
	/* pack the name to the buffer string,
	 * used in funct child_thrd_func
	 */
	char name_tmp[_MAX_DATA_LEN];
	memset(name_tmp, 0, sizeof(name_tmp));

	buffer[0] = NAME;
	buffer[1] = 1;
	gethostname(name_tmp, 255);
	strcat(buffer + _HEADER_LEN, name_tmp);
	strcat(buffer + _HEADER_LEN, "\0");
}
void pack_list(char* buffer) {
	/* pack the request list to the buffer string,
	 * used in funct child_thrd_func
	 */
	char list_tmp[_MAX_DATA_LEN];
	memset(list_tmp, 0, sizeof(list_tmp));

	buffer[0] = LIST;
	buffer[1] = 1;

	int i = 0;
	while (++i < _MAX_Q_SIZE) {
		if (req_list[i].dirty_bit == 1) {
			strcat(buffer + _HEADER_LEN, "Index: ");
			sprintf(list_tmp, "%d", i);
			strcat(buffer + _HEADER_LEN, list_tmp);
			strcat(buffer + _HEADER_LEN, "IP: ");
			strcat(buffer + _HEADER_LEN, inet_ntop(AF_INET, (void*)&req_list[i].soc_address.sin_addr, Buffer, 16));
			strcat(buffer + _HEADER_LEN, "port: ");
			sprintf(list_tmp, "%d", htons(req_list[i].soc_address.sin_port));
			strcat(buffer + _HEADER_LEN, list_tmp);
			strcat(buffer + _HEADER_LEN, "\n");
		}
	}
	return;
}
void send_data(char* req, char* resp) {
	/* if the request is send_data, we will forward the data as it request,
	 * we use the source client and the requested client to refer.
	 * used in funct child_thrd_func
	 */
	int src_seq = req[1];
	int ret;
	char tmp[_MAX_DATA_LEN];
	memset(tmp, 0, sizeof(tmp));

	if (req_list[src_seq].dirty_bit == 1) {
		//if found, then just send foward.
		ret = send(req_list[src_seq].curr_socket, req, strlen(req), 0);
		if (ret <= 0) {
			printf("send() to requested client fail, check the connect\n");
			strcat(resp, "send() to request client fail\n");
		}
		else {
			resp[0] = DATA;
			resp[1] = 1;
			strcat(resp + 2, "sent() to request client success\n");
		}
	}
	else {
		resp[0] = DATA;
		resp[1] = 1;
		strcat(resp + 2, "seems the client request by src client is not valid, check the request client send");
	}

}
