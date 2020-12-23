#include "server.h"
int main() {
	Ini_Socket();
	Bind();
	Listen();
	Create_main_thrd();
	return 0;
}