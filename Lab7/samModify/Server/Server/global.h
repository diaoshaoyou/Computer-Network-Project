#pragma once
//The max length of data
#define  MAXDATALENTH 128
//The listening port of server
#define  DAFAULTPORT 4345
//The type of request packet
typedef enum { CONNECT, EXIT, DISCONNECT, GETTIME, GETNAME, GETLIST, SENDMESSAGE }RequestOperation;