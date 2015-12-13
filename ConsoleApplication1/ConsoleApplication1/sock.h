#include <winsock2.h>

int SOCK_CONNECT(char *hostname, char * port, SOCKET *sock);
int SOCK_readyToReceive(int sock, int interval);
