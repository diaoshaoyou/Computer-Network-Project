#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Client.h"
#include "Error.h"
State state;//client state
HANDLE childReady;
HANDLE mainReady;
int socketID;
char sendData[BUF_SIZE];
char recvData[BUF_SIZE];

void createClient() {
	//init:
	state = State::DISCONN;
	memset(sendData, 0, sizeof(sendData));
	memset(recvData, 0, sizeof(recvData));

	WSADATA socketData;
	WORD wVersionRequested = MAKEWORD(2, 2);//use version 2.2, support multiprotocol
	if (WSAStartup(wVersionRequested, &socketData) != 0)//bind the socket
	{
		printf(BindError);
		return;
	}
	if (LOBYTE(socketData.wVersion) != 2 || HIBYTE(socketData.wVersion) != 2)//check the version
	{
		WSACleanup();
		printf(VersError);
		return;
	}
	//start main thread:
	HANDLE mainThr = NULL;
	DWORD  thrID = 0;
	mainThr = CreateThread(NULL, 0, mainThread, NULL, 0, &thrID);//create thread
	/*HANDLE CreateThread( 
	LPSECURITY_ATTRIBUTES lpThreadAttributes,//SD
	SIZE_T dwStackSize,//initialstacksize
	LPTHREAD_START_ROUTINE lpStartAddress,//threadfunction
	LPVOID lpParameter,//threadArgument
	DWORD dwCreationFlags,//creationOption
	LPDWORD lpThreadId//threadIdentifier
	)*/
	if (mainThr == NULL || thrID == 0)
	{
		printf(CreateError);
	}
}
DWORD WINAPI mainThread(LPVOID lpParam) {
	int choice = 0;
	while (1) {
		setUI();
		scanf("%d", &choice);
		switch (state) {
		case State::DISCONN:
			if (choice == 0) {//exit
				exit(0);
			}
			else {//connect
				Connect();
			}
			break;
		case State::CONN://0:exit, 1:get time, 2:get name, 3:get list
			switch (choice) {
			case 0://disconnect
				Disconnect();
				break;
			case 1://send get time request
				sendData[0] = TIME;
				Send();
				break;
			case 2://send get name request
				sendData[0] = NAME;
				Send();
			case 3://send get list request
				sendData[0] = LIST;
				Send();
				state = State::SEND;
				break;
			default:
				break;
			}
			break;
		case State::SEND:
			if (choice == 0) {//disconnect
				Disconnect();
			}
			else if (choice == 1) {//send data
				char* tmp = (char*)malloc(sizeof(char) * BUF_SIZE);
				printf("Please input your sending user ID:\n");
				scanf("%s", tmp);
				while (strlen(tmp) > MAX_CLIENT) {//user ID is wrong
					printf(IDError);
					scanf("%s", tmp);
				}
				strcpy(sendData + 1, tmp);
				printf("Please input your sending data:\n");
				scanf("%s", tmp);
				while (strlen(tmp) > MAX_DATA || strlen(tmp) == 0) {//data is too long or empty
					printf(DataError);
					scanf("%s", tmp);
				}
				strcpy(sendData + MAX_CLIENT + 1, tmp);
				strcat(sendData + MAX_CLIENT + 1, "\0");
				sendData[0] = DATA;
				Send();
			}
			else {//choice==2
				state = State::CONN;
				continue;
			}
			break;
		default:
			break;
		}
	}
	if (state!=State::DISCONN){
		if (WaitForSingleObject(childReady, 10) == WAIT_OBJECT_0){//main thread waits 10ms for child thread to be ready
		/*DWORD WaitForSingleObject( HANDLE hHandle, DWORD dwMilliseconds);*/
			ResetEvent(childReady);//unsignal childReady
			Sleep(1);
		}
	}
}

void setUI() {
	switch (state) {
	case State::DISCONN:
		printf("0. Exit\n1. Connect to Server\n");
		printf("Please choose the order number: ");
		break;
	case State::CONN://0:exit, 1:get time, 2:get name, 3:get list
		printf("0. Disconnect\n1. Get time\n2. Get name\n3. Get list\n");
		printf("Please choose the order number: ");
		break;
	case State::SEND:
		printf("0. Disconnect\n1. Send message\n2. Back\n");
		printf("Please choose the order number: ");
		break;
	default:
		break;
	}
}
void Disconnect() {
	state = State::DISCONN;
	sendData[0] = DISCONN;
	if (Send() == -1) {
		printf(DisconnError);
	}
	else {
		printf(SUCCESS);
	}
}
DWORD WINAPI childThread(LPVOID lpParam) {//thread for receiving data
	while (1) {
		if (recv(socketID, recvData, BUF_SIZE, 0) <= 0) {//?
			printf(RecvError);
			break;
		}
		else {
			printf("%s\n", recvData);
			SetEvent(childReady);//make child thread ready
		}
	}
	closesocket(socketID);
	return 0;
}
void Connect() {//connect to server: create socket+connect+create child thread
	struct sockaddr_in serverAddr;
	//serverAddr = (struct sockaddr_in*)malloc(sizeof(sockaddr_in));
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(IP);
	serverAddr.sin_family = AF_INET;

	socketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//create socket
	if (socketID < 0) {
		printf(CreateError);
		return;
	}
	int tmp = connect(socketID, (const struct sockaddr*)&serverAddr, sizeof(sockaddr));
	/*int connect(SOCKET s, const struct sockaddr * name, int namelen);*/
	if (tmp<0) {
		printf(ConnError);
		return;
	}
	else if (tmp == ETIMEDOUT) {
		printf(TimeoutError);
		return;
	}
	mainReady = CreateEvent(NULL, false, true, NULL);
	state = State::CONN;

	/*HANDLE     CreateEvent(
    LPSECURITY_ATTRIBUTES     lpEventAttributes,     //SD   
    BOOL     bManualReset,      //true:person set false; false:auto set  
    BOOL     bInitialState,     //initial state   
    LPCTSTR     lpName          //object name   
    );   */
	//create child thread
	HANDLE childThr = NULL;
	DWORD  thrID = 0;
	childThr = CreateThread(NULL, 0, childThread, NULL, 0, &thrID);//create thread
	if (childThr == NULL || thrID == 0)
		printf(CreateError);
	childReady = CreateEvent(NULL, false, true, NULL);
	printf(SUCCESS);
}

int Send() {//send request
	if (sendData[0] != DATA) {
		for (int i = 1; i < BUF_SIZE; i++) {
			sendData[i] = '0';
		}
	}
	if (send(socketID, (const char*)sendData, strlen(sendData), 0)<0) {
		/*ssize_t send(int sockfd, const void *buf, size_t len, int flags);*/
		printf(SendError);
		return -1;
	}
	else {
		printf(SUCCESS);
	}
}
