#pragma once
#define HEADERLENGTH 2//The header,s length
#define  MAXDATALENTH 128//The max length of data
#define  DAFAULT_PORT 3574//Default listening port of server
//The type of request packet
typedef enum { CONN, EXIT, DISCONN, TIME, NAME, LIST, SENDMSG }RequestOperation;
//The port of server
int ServerPort = DAFAULT_PORT;
//print tips:
#define WSASErr "$Server Error: WSAStartup() failed!\n"
#define VersionErr "$Invalid Winsock version!\n"
#define AcceptErr "$Accept error!"
#define RangeErr "$Exceeding the max amount of queue..."
#define ChildThrErr "$Creat Child Thread failed.\n"
#define MainThrErr "$CreatThread failed.\n"
#define CreateSockErr "$Socket creation failed!\n"
#define ConnErr "$Socket connection failed!\n"
#define BindErr "$Server bind failed!\n"
#define RecvErr "$Recv failed!\n"
#define CreateChild "$ChildThread %4d created!\n"
#define CreateMain "$Welcome to Server: MainThread %4d created!\n"
#define ServListen "$Listening on port 3574\n"
#define SendErr "$Send request failed!\n"
#define InputIndex "$Please input the client's index which you want to send message:\n"
#define InputMsg "$Please input the message you want to send:\n"
#define InputIP "$Please type in your target server's IP:\n"
#define ConnFirstErr "$You are not connected yet. Please connect first!\n"
#define ListenErr "#Listen failed with error: %ld\n"
#define SendSuccess "$Send to target client successfully!\n"
#define AlreadyConn "$You have already connect to a server,please disconnect before connecting to a new server!\n"
#define S_ContactErr "#Sending to target client failed!\n"//server's error of clients contaction
#define C_ContactErr "$Sending to target client failed!\n"//client's error of clients contaction
#define NoTargetErr "$No target client!\n"
#define C_preface "Welcome!\n\
0.  Connect To Server\n\
1.  Exit\n\
2.  Disconnect from Server\n\
3.  Get current time\n\
4.  Get server's name\n\
5.  Get clients' list\n\
6.  Send message to another clients\n\
----------------------------------------\n"

#define C_InputTip "$Please input the operation's index:\n"
