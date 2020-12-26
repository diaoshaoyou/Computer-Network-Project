#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "client.h"
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
		printf("CreatThread failed.\n");
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
		printf("WSAStartup() failed!\n");
		return;
	}

	//Check the version
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return;
	}

}

//The function to be called by mai  thread
DWORD WINAPI mainThrFun(LPVOID lp) {
	//Display the start menu
	int Op = -1;
	printf("Main thread ID:%4d created!\n", GetCurrentThreadId());

	printf("        Welcome To Socket Client!       \n");
	printf("**************Start Menu*************** *\n");
	printf("* 0.  Connect To Server                 *\n");
	printf("* 1.  Exit                              *\n");
	printf("* 2.  Disconnect from Server            *\n");
	printf("* 3.  Get current time                  *\n");
	printf("* 4.  Get server's name                 *\n");
	printf("* 5.  Get clients' list                 *\n");
	printf("* 6.  Send message to another clients   *\n");
	printf("*****************************************\n");

	while (1)
	{
		printf("Please input the operation's index:     \n");
		printf("client@:");
		//Input the instruction
		scanf("%d", &Op);
		switch (Op)
		{
		case CONNECT:
			connect_to_server();
			break;
		case EXIT:
			exit();
			break;
		case DISCONNECT:
			disconnect();
			break;
		case GETTIME:
			send_time_request();
			break;
		case GETNAME:
			send_name_request();
			break;
		case GETLIST:
			send_list_request();
			break;
		case SENDMESSAGE:
			send_message_request();
			break;
		default:
			//printf("Invalid input!\n");
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
void connect_to_server(void) {

	//If the connection is established,return
	if (ConnectStatus)
	{
		printf("You have already connect to a server,please disconnect before connecting to a new server!\n");
		return;
	}
	//Input the server's IP address and port
	printf("Please type in your target server's IP:\n");
	scanf("%s", ServerIP);
	printf("Please type in your target server's Port:");
	scanf("%d", &ServerPort);
	//Select the valid port
	while (ServerPort <= 1024 && ServerPort <= 65535)
	{
		printf("Invalid port number,please select a port range from 1025 to 65534!\n");
		printf("Please type in your target server's Port:  ");
		scanf("%d", &ServerPort);
	}

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
		printf("Client socket() failed!\n");
		return;
	}
	printf("SOCKET: %lu\n", ClientSocket);

	//Create InputEvent's event
	InputEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	int ret = connect(ClientSocket, (PSOCKADDR)&Server, sizeof(Server));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}
	//Set the status to be true
	ConnectStatus = TRUE;

	//Create child thread
	ReceiveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Create child thread
	create_child_thread();
}

//Create child thread
void create_child_thread(void) {
	HANDLE ChildThread = NULL;
	DWORD  ThreadId = 0;

	//Call the function
	ChildThread = CreateThread(NULL, 0, childThrFun, NULL, 0, &ThreadId);
	if (ChildThread == NULL || ThreadId == 0)
	{
		printf("Creat Child Thread failed.\n");
	}
}

//The function to be called by child thread
DWORD WINAPI childThrFun(LPVOID lp) {
	printf("SubThread_Receiving ID:%4d created!\n", GetCurrentThreadId());
	int ret = 0;

	//Receive the response from the server
	while (true)
	{
		memset(ReceiveBuf, 0, sizeof(ReceiveBuf));
		ret = recv(ClientSocket, ReceiveBuf, sizeof(ReceiveBuf), 0);


		if (ret <= 0)
		{
			printf("recv failed!\n");
			break;
		}

		printf("%s", ReceiveBuf + 2);
		printf("\n");

		SetEvent(ReceiveEvent);
	}

	printf("SubThread ID:%d stop!\n", GetCurrentThreadId());
	closesocket(ClientSocket);
	return 1;
}

//Exit the program
void exit(void) {
	//Connect from the server first
	if (ConnectStatus)
	{
		disconnect();
	}
	exit(0);
}

//Disconnect from the server
void disconnect(void) {
	if (ConnectStatus == false)
	{
		printf("You are not connected yet!\n");
		printf("Please connect to server first!\n");
	}
	else
	{
		//Send a request to the server the clinet will disconnect
		ConnectStatus = 0;
		memset(RequestBuf, 0, sizeof(RequestBuf));
		RequestBuf[0] = DISCONNECT;
		RequestBuf[1] = 1;
		RequestBuf[2] = 0;
		int ret = send(ClientSocket, RequestBuf, strlen(RequestBuf), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Disconnect from server  failed!\n");
			printf("Please connect to server first!\n");
		}

	}
}

//Send a request to get current time
void send_time_request(void) {
	//Check if the connection has established
	if (ConnectStatus == false)
	{
		printf("You are not connected yet!\n");
		printf("Please connect to server first!\n");
	}
	else
	{
		//Ask for the current time
		for (int j = 0; j < 100; j++)
		{
			memset(RequestBuf, 0, sizeof(RequestBuf));
			RequestBuf[0] = GETTIME;
			RequestBuf[1] = 1;
			RequestBuf[2] = 0;

			int ret = send(ClientSocket, RequestBuf, strlen(RequestBuf), 0);
			if (ret == SOCKET_ERROR)
			{
				printf("Send time request failed!\n");
			}
		}

	}
}

//Send a request to get the server's name
void send_name_request(void) {
	//Check if the connection has established
	if (ConnectStatus == false)
	{
		printf("You are not connected yet!\n");
		printf("Please connect to server first!\n");
	}
	else
	{
		//Ask for the server's name
		memset(RequestBuf, 0, sizeof(RequestBuf));
		RequestBuf[0] = GETNAME;
		RequestBuf[1] = 1;
		RequestBuf[2] = 0;

		int ret = send(ClientSocket, RequestBuf, strlen(RequestBuf), 0);

		if (ret == SOCKET_ERROR)
		{
			printf("Send name request failed!\n");
		}
	}
}

//Send a request to get the clients' list
void send_list_request(void) {
	//Check if the connection has established
	if (ConnectStatus == false)
	{
		printf("You are not connected yet!\n");
		printf("Please connect to server first!\n");
	}
	else
	{
		//Ask for the clients' list
		memset(RequestBuf, 0, sizeof(RequestBuf));
		RequestBuf[0] = GETLIST;
		RequestBuf[1] = 1;
		RequestBuf[2] = 0;
		printf("\nSendList\n!");
		int ret = send(ClientSocket, RequestBuf, strlen(RequestBuf), 0);

		if (ret == SOCKET_ERROR)
		{
			printf("Send list request failed!\n");
		}
	}
}
//Send a message to another client
void send_message_request(void) {
	int index = 1;
	//Check if the connection has established
	if (ConnectStatus == false)
	{
		printf("You are not connected yet!\n");
		printf("Please connect to server first!\n");
	}
	else
	{
		memset(RequestBuf, 0, sizeof(RequestBuf));
		//Input the client's index you want to send
		printf("Please input the client's index which you want to send message:\n");
		scanf("%d", &index);
		//Input the client's message you want to send
		printf("Please input the message you want to send\n");
		scanf("%s", RequestBuf + 2);
		strcat(RequestBuf + 2, "\0");

		RequestBuf[0] = SENDMESSAGE;
		RequestBuf[1] = index;
		int ret = send(ClientSocket, RequestBuf, strlen(RequestBuf), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Send message request failed!\n");
		}
	}
}
