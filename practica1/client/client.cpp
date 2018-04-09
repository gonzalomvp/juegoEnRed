// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

#define SERVER_PORT "12345"
#define BUF_SIZE 4096
int main(int argc, char *argv[])
{
	WSADATA wsaData;
	int wsaerr;
	SOCKET sockfd;
	char buf[BUF_SIZE];
	memset(&buf, 0, sizeof(buf));
	if (argc < 2) return -1;
	wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0)
	{
		printf("Could not initialize Winsock.\n");
		return -1;
	}
	struct addrinfo hints;// hints de la direccion buscada
	struct addrinfo* servInfo;// contendra la lista de direccciones encontradas
	struct addrinfo* srvaddr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// Busca las addrinfo del host al que nos queremos conectar
	int error = getaddrinfo(argv[1], SERVER_PORT, &hints, &servInfo);

	if (error != 0)
	{
		printf("getaddrinfo failed");
		WSACleanup();
		return -1;
	}
	for (srvaddr = servInfo; (srvaddr != NULL); srvaddr = srvaddr->ai_next)
	{
		sockfd = socket(srvaddr->ai_family, srvaddr->ai_socktype, srvaddr->ai_protocol);
		if (sockfd != INVALID_SOCKET) {
			int iResult = connect(sockfd, srvaddr->ai_addr, (int)srvaddr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(sockfd);
				sockfd = INVALID_SOCKET;
			}
			else
				break;
		}
	}
	freeaddrinfo(servInfo);
	if (sockfd != INVALID_SOCKET)
	{
		gets_s(buf, _countof(buf));
		int length = strlen(buf) + 1;

		int numBytesSend, totalSend = 0;
		do
		{
			numBytesSend = send(sockfd, buf, length - totalSend, 0);
			totalSend += numBytesSend;
		} while (totalSend < length);
		closesocket(sockfd);
	}
	WSACleanup();

    return 0;
}

