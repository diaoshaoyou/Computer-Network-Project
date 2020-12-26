#pragma once
#ifndef CLIENT_H
#define CLIENT_H
#define IP "10.1.1.1"
#define MAX_DATA 256//max packet size=256 bytes
#define BUF_SIZE 260//sizeof(sendData)
#define MAX_CLIENT 2//max client size = "99" - 1 = 98
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "global.h"

enum class State{//state
	DISCONN,
	CONN,
	SEND
};


void createClient();
DWORD WINAPI mainThread(LPVOID lpParam);//must be global or static function
DWORD WINAPI childThread(LPVOID lpParam);//must be global or static function
void setUI();
void Disconnect();
void Connect();
int Send();

#endif // !CLIENT_H