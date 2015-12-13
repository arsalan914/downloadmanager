#include "sock.h"
#include <ws2tcpip.h>
#include <stdio.h>
#include <Time.h>

/*
TODO: handle case where hostname is IP.
*/
int SOCK_CONNECT(char *hostname, char * port, SOCKET *sock)
{
	struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
	int iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(hostname, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		*sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (*sock == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(*sock, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			closesocket(*sock);
			*sock = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	return 0;

}

int SOCK_readyToReceive(int sock, int interval )
{
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec = interval;
	tv.tv_usec = 0;

	return (select(sock + 1, &fds, 0, 0, &tv) == 1);
}
