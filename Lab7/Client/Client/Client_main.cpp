#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "client.h"
/*
*Only support sending data once!! When sending data the second time, please input "6" again.
*/
int main(void) {
	//Initialize socket
	InitSocket();
	//Create main thread
	HANDLE MainThread = NULL;
	DWORD  ThreadId = 0;

	//Call the function  mainThrFun
	MainThread = CreateThread(NULL, 0, mainThrFun, NULL, 0, &ThreadId);
	if (MainThread == NULL || ThreadId == 0)
	{
		printf(MainThrErr);
	}

	//Loop for the main thread
	while (1)
	{
		if (WaitForSingleObject(InputEvent, 10) == WAIT_OBJECT_0)
		{
			ResetEvent(InputEvent);
			Sleep(1);
		}
	}
	return 0;

}
//Initialize socket
void InitSocket(void) {
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		printf(WSASErr);
		return;
	}

	//Check the version
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf(VersionErr);
		return;
	}

}

//The function to be called by mai  thread
DWORD WINAPI mainThrFun(LPVOID lp) {
	//Display the start menu
	int Op = -1;
	printf(CreateMain, GetCurrentThreadId());

	printf(C_preface);
	int index = 1;
	while (1)
	{
		if (recvJustNow) {
			printf(C_InputTip);
			recvJustNow = false;
		}
		//Input the instruction
		scanf("%d", &Op);
		switch (Op)
		{
		case CONN:
			_connect();
			break;
		case EXIT:
			exit();
			break;
		case DISCONN:
			disconnect();
			break;
		case TIME:
			//send_time_request();
			memset(RequestBuf, 0, sizeof(RequestBuf));
			RequestBuf[0] = TIME;
			//for (int i = 0; i < 4; i++)//debug
			sendRequest();
			break;
		case NAME:
			memset(RequestBuf, 0, sizeof(RequestBuf));
			RequestBuf[0] = NAME;
			sendRequest();
			break;
		case LIST:
			memset(RequestBuf, 0, sizeof(RequestBuf));
			RequestBuf[0] = LIST;
			sendRequest();
			break;
		case SENDMSG:
			memset(RequestBuf, 0, sizeof(RequestBuf));
			printf(InputIndex);//Input the client's index you want to send
			scanf("%d", &index);
			printf(InputMsg);//Input the client's message you want to send
			scanf("%s", RequestBuf + HEADERLENGTH);
			strcat(RequestBuf + HEADERLENGTH, "\0");
			RequestBuf[0] = SENDMSG;
			RequestBuf[1] = index;
			sendRequest();
			break;
		default:
			break;
		}

		//If the connectton is established,creating the child thread to receive message from the server
		if (ConnectStatus)
		{
			if (WaitForSingleObject(ReceiveEvent, 10) == WAIT_OBJECT_0)
			{
				ResetEvent(ReceiveEvent);
				Sleep(1);
			}
		}

	}

}

//The opertion to connect the client and the server
void _connect(void) {

	//If the connection is established,return
	if (ConnectStatus)
	{
		printf(AlreadyConn);
		recvJustNow = true;
		return;
	}
	//Input the server's IP address
	printf(InputIP);
	scanf("%s", ServerIP);

	//Initialize the server
	SOCKADDR_IN Server;
	Server.sin_family = AF_INET;
	Server.sin_port = htons(ServerPort);
	Server.sin_addr.s_addr = inet_addr(ServerIP);

	//Get the socket
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		WSACleanup();
		printf(CreateSockErr);
		return;
	}
	printf("$Socket: %lu, Port: %d, IP: %s\n", ClientSocket, ServerPort, ServerIP);

	//Create InputEvent's event
	InputEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	int ret = connect(ClientSocket, (PSOCKADDR)&Server, sizeof(Server));
	if (ret == SOCKET_ERROR)
	{
		printf(ConnErr);
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}
	//Set the status to be true
	ConnectStatus = true;

	//Create child thread
	ReceiveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Create child thread
	create_child_thread();
}

//Create child thread
void create_child_thread() {
	HANDLE ChildThread = NULL;
	DWORD  ThreadId = 0;

	//Call the function
	ChildThread = CreateThread(NULL, 0, childThrFun, NULL, 0, &ThreadId);
	if (ChildThread == NULL || ThreadId == 0)
	{
		printf(ChildThrErr);
	}
}

//The function to be called by child thread
DWORD WINAPI childThrFun(LPVOID lp) {
	printf(CreateChild, GetCurrentThreadId());
	int ret = 0;

	//Receive the response from the server
	while (true)
	{
		memset(ReceiveBuf, 0, sizeof(ReceiveBuf));
		if (recv(ClientSocket, ReceiveBuf, sizeof(ReceiveBuf), 0) <= 0)
		{
			printf(RecvErr);
			break;
		}
		printf("$Received-->%s", ReceiveBuf + HEADERLENGTH);
		printf("\n");
		recvJustNow = true;//print input tips after reciving
		SetEvent(ReceiveEvent);
		if (ConnectStatus == false) {
			break;
		}
	}
	closesocket(ClientSocket);
	return 1;
}

//Exit the program
void exit(void) {
	if (ConnectStatus)
	{
		disconnect();
	}
	exit(0);
}
void disconnect() {
	memset(RequestBuf, 0, sizeof(RequestBuf));
	RequestBuf[0] = DISCONN;
	sendRequest();
}

void sendRequest() {
	if (ConnectStatus == false)
		printf(ConnFirstErr);
	else {
		if(RequestBuf[0]==DISCONN)
			ConnectStatus = false;
		if (RequestBuf[0] != SENDMSG) {
			RequestBuf[1] = 1;
			RequestBuf[2] = 0;
		}
		if (send(ClientSocket, RequestBuf, strlen(RequestBuf), 0) < 0) {
			printf(SendErr);
		}
	}
}
