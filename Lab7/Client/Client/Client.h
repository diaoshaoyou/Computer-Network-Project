#pragma once
#ifndef CLIENT_H
#define CLIENT_H
#define PORT 8000
#define IP "10.1.1.1"
#define MAX_DATA 256//max packet size=256 bytes
#define BUF_SIZE 260//sizeof(sendData)
#define MAX_CLIENT 2//max client size = "99" - 1 = 98
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

enum class State{//state
	DISCONN,
	CONN,
	SEND
};
enum PacketRequest {//request packet type
	NAME='1',
	LIST='2',
	TIME='3',
	DATA='4'
};

class Client {
public:
	Client();
	static DWORD WINAPI mainThread(LPVOID lpParam);//must be global or static function
	static DWORD WINAPI childThread(LPVOID lpParam);//must be global or static function
	static void setUI();
	static void Disconnect();
	static void Connect();
	static void setRequest();
};
#endif // !CLIENT_H