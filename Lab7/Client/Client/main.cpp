#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Client.h" 
/*packet structure:
 *packet type(1 byte) + sending object(1 byte) + data(256 byte most)
 *packet type: NAME='1' LIST='2' TIME='3' DATA='4'
 *sending object: server='0' other users>'0'
*/
int main() {
	Client client();
	/*char* tmp = (char*)malloc(sizeof(char) * 244);
	scanf("%s", tmp);
	printf("%d", strlen(tmp));*/
	return 0;
}

