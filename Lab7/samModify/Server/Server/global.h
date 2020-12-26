#pragma once
//The max length of data
#define  MAXDATALENTH 128
#define  DAFAULT_PORT 3574//Default listening port of server
#define DEFAULT_IP "10.1.1.1"//Default server IP
//The type of request packet
typedef enum { CONN, EXIT, DISCONN, TIME, NAME, LIST, SENDMSG }RequestOperation;
//The port of server
int ServerPort = DAFAULT_PORT;
//print tips:
#define WSASErr "Server Error: WSAStartup() failed!\n"
#define VersionErr "Invalid Winsock version!\n"
#define AcceptErr "accept error!"
#define RangeErr "Exceeding the max amount of queue..."
#define ChildThrErr "Creat Child Thread failed.\n"
#define CreateSockErr "Socket creation failed!\n"
#define ConnErr "Socket connection failed!\n"
#define BindErr "Server bind failed!\n"
#define RecvErr "recv failed!\n"
#define CreateChild "ChildThread %4d created!\n"
#define CreateMain "Welcome to Server: MainThread %4d created!\n"
#define ServListen "Listening on port 3574"

#define C_preface "Welcome!\n\
0.  Connect To Server\n\
1.  Exit\n\
2.  Disconnect from Server\n\
3.  Get current time\n\
4.  Get server's name\n\
5.  Get clients' list\n\
6.  Send message to another clients\n\
*****************************************\n"

#define C_InputTip "Please input the operation's index:     \n$"