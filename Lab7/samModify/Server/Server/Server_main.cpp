#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"
int main(void) {
	//Initialize socket
	InitSocket();
	//bind
	BindSocket();
	
	Listen();//listen on port 
	//create the main thread
	CreateMainThr();
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

	//Initialize the clients' socket
	for (int i = 0; i < MAXQUEUE; i++) {
		ClientList[i].DirtyBit = 0;
	}
	//printf("Initializing success!\n");
}

void CreateMainThr(void) {
	printf(CreateMain, GetCurrentThreadId());
	int i = 0;
	ChildClient CurrentChild;
	while (true)
	{
		printf(ServListen);
		//Accept the request from the client
		ClientSocket = accept(ServerSocket, (SOCKADDR*)(&addr), &addrlen);
		if (ClientSocket == INVALID_SOCKET)
		{
			printf(AcceptErr);
			break;
		}
		i = 0;
		printf("\n");
		while (++i < MAXQUEUE)
		{
			//Allocate a empty part for the new client
			if (ClientList[i].DirtyBit == 0)
			{
				CurrentChild.ChildSocket = ClientSocket;
				CurrentChild.index = i;
				ClientList[i].CurrentSocket = ClientSocket;
				ClientList[i].DirtyBit = 1;
				ClientList[i].SocAddress = addr;
				ClientList[i].ChildThread = createChildThr(&CurrentChild);

				printf("#Client %d log in.\n", i);
				printf("#Client %d's ip address is %s.\n", i, inet_ntop(AF_INET, (void*)&addr.sin_addr, Buffer, 16));
				printf("#Client %d's port is %d.\n", i, htons(addr.sin_port));
				printf("#Client %d's socket is  %d\n", i, ClientSocket);
				break;
			}

		}
		//If the clients' number reach to MAXQUEUE,allocating failed
		if (i == MAXQUEUE)
		{
			printf(RangeErr);
			closesocket(ClientSocket);
		}
	}
	closesocket(ServerSocket);
	//printf("main_thread_func,break\n");
	WSACleanup();

}

//Create a child thread
HANDLE createChildThr(ChildClient* ChildPtr) {
	HANDLE ChildThread = NULL;
	DWORD  ThreadId = 0;
	ChildThread = CreateThread(NULL, 0, childThrFun, (LPVOID)ChildPtr, 0, &ThreadId);
	if (ChildThread == NULL || ThreadId == 0)
	{
		printf(ChildThrErr);
	}
	return  ChildThread;
}
//The function to be called by the child thread
DWORD WINAPI childThrFun(LPVOID lp) {
	printf(CreateChild, GetCurrentThreadId());
	char RequestBuffer[MAXDATALENTH];
	char ResponseBuffer[MAXDATALENTH];
	char IndexTemp[10];
	int ret = 0;
	ChildClient* Ptr = (ChildClient*)lp;
	SOCKET CurrentSocket = Ptr->ChildSocket;
	int ChildIndex = Ptr->index;
	int ConnectStatus = 1;
	int Op = 0;


	//Say hello to the client
	memset(ResponseBuffer, 0, sizeof(ResponseBuffer));
	memset(RequestBuffer, 0, sizeof(RequestBuffer));
	memset(IndexTemp, 0, sizeof(IndexTemp));
	sprintf(IndexTemp, "%d", ChildIndex);
	strcat(ResponseBuffer + 2, "Hello! Client ");
	strcat(ResponseBuffer + 2, IndexTemp);
	strcat(ResponseBuffer + 2, "\n----------------------------------------");
	//strcat(ResponseBuffer + 2, "  You have connected to the server successfully!");
	if (send(CurrentSocket, ResponseBuffer, sizeof(ResponseBuffer), 0) <= 0)
	{
		printf(RecvErr);
	}
	while (1)
	{
		memset(ResponseBuffer, 0, sizeof(ResponseBuffer));
		memset(RequestBuffer, 0, sizeof(RequestBuffer));
		//Receive message from the current client's socket
		ret = recv(CurrentSocket, RequestBuffer, sizeof(RequestBuffer), 0);

		if (ret <= 0)
		{
			printf(RecvErr);
			break;
		}
		Op = RequestBuffer[0];
		/*if (Op == DISCONN)
		{
			ClientList[ChildIndex].DirtyBit = 0;
			printf("Client %d log out!\n", ChildIndex);
			closesocket(CurrentSocket);
			break;
		}*/
		//Response different message to the client
		switch (Op)
		{
		case DISCONN:
			ClientList[ChildIndex].DirtyBit = 0;
			printf("#Client %d log out!\n", ChildIndex);
			ResponseBuffer[0] = DISCONN;
			ResponseBuffer[1] = 1;
			strcat(ResponseBuffer + HEADERLENGTH, "Goodbye!\n\0");//send goodbye
			send(CurrentSocket, ResponseBuffer, sizeof(ResponseBuffer), 0);
			closesocket(CurrentSocket);
			return 0;
		case TIME:
			pack_time(ResponseBuffer);
			break;
		case NAME:
			pack_name(ResponseBuffer);
			break;
		case LIST:
			pack_list(ResponseBuffer);
			break;
		case SENDMSG:
			send_message(RequestBuffer, ResponseBuffer);
			break;
		default:
			break;
		}
		printf("#Response to client %d: %s\n", ChildIndex, ResponseBuffer + HEADERLENGTH);
		ret = send(CurrentSocket, ResponseBuffer, sizeof(ResponseBuffer), 0);
	}
	return 0;
}
//Get and reply current time
void pack_time(char* buffer) {
	time_t t;
	struct tm* lt;
	time(&t);
	lt = localtime(&t);
	char temp[MAXDATALENTH] = { 0 };
	memset(temp, 0, sizeof(temp));

	buffer[0] = TIME;
	buffer[1] = 1;
	sprintf(temp, "%d", lt->tm_year + 1900);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, "/");
	sprintf(temp, "%d", lt->tm_mon + 1);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, "/");
	sprintf(temp, "%d", lt->tm_mday);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, " ");

	sprintf(temp, "%d", lt->tm_hour);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, ":");
	sprintf(temp, "%d", lt->tm_min);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, ":");
	sprintf(temp, "%d", lt->tm_sec);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, "\0");

}
//Get and reply server's name
void pack_name(char* buffer) {
	char temp[MAXDATALENTH] = { 0 };
	memset(temp, 0, sizeof(temp));

	buffer[0] = NAME;
	buffer[1] = 1;
	gethostname(temp, 255);
	strcat(buffer + HEADERLENGTH, temp);
	strcat(buffer + HEADERLENGTH, "\0");
}
//Get and reply clients' list
void pack_list(char* buffer) {
	char temp[MAXDATALENTH] = { 0 };
	char TempAddr[MAXDATALENTH] = { 0 };
	int i = 0;
	memset(temp, 0, sizeof(temp));

	buffer[0] = LIST;
	buffer[1] = 1;
	while (++i < MAXQUEUE)
	{
		if (ClientList[i].DirtyBit == 1)
		{
			printf("#Find Client %d\n", i);
			strcat(buffer + HEADERLENGTH, "Index: ");
			sprintf(temp, "%d ", i);
			strcat(buffer + HEADERLENGTH, temp);
			strcat(buffer + HEADERLENGTH, "IP : ");
			strcat(buffer + HEADERLENGTH, inet_ntop(AF_INET, (void*)&ClientList[i].SocAddress.sin_addr, Buffer, 16));
			strcat(buffer + HEADERLENGTH, " port : ");
			sprintf(temp, "%d", htons(ClientList[i].SocAddress.sin_port));
			strcat(buffer + HEADERLENGTH, temp);
			strcat(buffer + HEADERLENGTH, "\n");
		}
	}

}
//Send a message to destination client
void send_message(char* request, char* reply) {
	int SrcIndex = request[1];
	int ret;
	char temp[MAXDATALENTH] = { 0 };
	//Find if the client is connected or not.
	if (ClientList[SrcIndex].DirtyBit)
	{
		printf("#Request: %s\n", request + HEADERLENGTH);
		ret = send(ClientList[SrcIndex].CurrentSocket, request, strlen(request), 0);
		if (ret <= 0)
		{
			printf(S_ContactErr);
			strcat(reply, C_ContactErr);
		}
		else
		{
			reply[0] = SENDMSG;
			reply[1] = 1;
			strcat(reply + HEADERLENGTH, SendSuccess);
		}

	}
	else
	{
		reply[0] = SENDMSG;
		reply[1] = 1;
		strcat(reply + HEADERLENGTH, NoTargetErr);
	}

}

//Bind the IP address and port
void BindSocket(void) {
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (ServerSocket == INVALID_SOCKET)
	{
		WSACleanup();
		printf(CreateSockErr);
		return; //TODO
	}

	int len = sizeof(SOCKADDR);
	SOCKADDR_IN Server;
	Server.sin_family = AF_INET;
	Server.sin_port = htons(DAFAULT_PORT);
	Server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(ServerSocket, (SOCKADDR*)&Server, len) == INVALID_SOCKET)
	{
		printf(BindErr);
		closesocket(ServerSocket);
		WSACleanup();
		return;
	}
	//printf("Binding success!\n");
}
//Listening on port 4345
void Listen(void) {

	if (listen(ServerSocket, MAXQUEUE) == SOCKET_ERROR) {
		printf(ListenErr, WSAGetLastError());
		closesocket(ServerSocket);
		WSACleanup();
		return;
	}
	//printf("Listening success!\n");
}
