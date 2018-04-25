// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

#include <vector>


std::vector<SOCKET*> g_clients;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void* receiveMessages(void* argument);
void sendMessageToClients(const char* message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	int wsaerr;
	SOCKET sockId, socketCliId;
	int on = 1;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0) {
		printf("Could not initialize Winsock.\n");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // Aquí NO se usa htons.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	sockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockId < 0) {
		printf("Error creating socket\n");
		WSACleanup();
		return -1;
	}
	setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

	if (bind(sockId, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("Error binding port\n");
		WSACleanup();
		return -1;
	}
	if (listen(sockId, QUEUE_SIZE) < 0) {
		printf("listen failed\n");
		WSACleanup();
		return -1;
	}

	while (1) {
		socklen_t sock_len = sizeof(client);
		memset(&client, 0, sizeof(client));
		socketCliId = accept(sockId, (struct sockaddr *)&client, &sock_len);
		if (socketCliId == INVALID_SOCKET)
			printf("Can't accept client:%d\n", WSAGetLastError());
		else {
			SOCKET* socket = new SOCKET(socketCliId);
			pthread_t receiveMessagesThread;
			if (pthread_create(&receiveMessagesThread, NULL, receiveMessages, socket)) {

				fprintf(stderr, "Error creating thread\n");
				WSACleanup();
				return 1;
			}
		}
	}
	closesocket(sockId);
	WSACleanup();

    return 0;
}

void* receiveMessages(void* argument) {
	char buf[BUF_SIZE];
	char messageToSend[BUF_SIZE];
	char nick[BUF_SIZE];
	int rec = 0;
	int totalRecv = 0;

	SOCKET* socketCliId = (SOCKET*)argument;

	//receive nick
	totalRecv = 0;
	do {
		rec = recv(*socketCliId, buf + totalRecv, BUF_SIZE, 0);
		totalRecv += rec;
	} while (rec != -1 && buf[totalRecv - 1] != '\0');

	if (rec == -1) {
		closesocket(*socketCliId);
		delete socketCliId;
		return nullptr;
	}

	strcpy_s(nick, buf);
	printf("%s is now connected\n", nick);
	
	pthread_mutex_lock(&g_mutex);
	g_clients.push_back(socketCliId);
	pthread_mutex_unlock(&g_mutex);

	strcpy_s(messageToSend, nick);
	strcat_s(messageToSend, " is now connected");
	sendMessageToClients(messageToSend);

	//keep receiving from client
	while (socketCliId) {
		totalRecv = 0;
		do {
			rec = recv(*socketCliId, buf + totalRecv, BUF_SIZE, 0);
			totalRecv += rec;
		} while (rec != -1 && buf[totalRecv - 1] != '\0');

		if (rec != -1) {
			printf("%s> %s\n", nick, buf);
			strcpy_s(messageToSend, nick);
			strcat_s(messageToSend, "> ");
			strcat_s(messageToSend, buf);

			//Resend received message
			sendMessageToClients(messageToSend);
		}
		else {
			pthread_mutex_lock(&g_mutex);
			for (auto itClients = g_clients.begin(); itClients != g_clients.end(); ++itClients) {
				if (*itClients == socketCliId) {
					g_clients.erase(itClients);
					break;
				}
			}
			pthread_mutex_unlock(&g_mutex);
			
			closesocket(*socketCliId);
			delete socketCliId;
			socketCliId = nullptr;

			//mensaje de desconexion
			printf("%s has been disconnected\n", nick);
			strcpy_s(messageToSend, nick);
			strcat_s(messageToSend, " has been disconnected");
			sendMessageToClients(messageToSend);
		}
	}

	return nullptr;
}

void sendMessageToClients(const char* message) {
	std::vector<SOCKET*> clientsToRemove;
	int length = strlen(message) + 1;
	pthread_mutex_lock(&g_mutex);
	for (size_t i = 0; i < g_clients.size(); i++)
	{
		int numBytesSend, totalSend = 0;
		do
		{
			numBytesSend = send(*g_clients[i], message, length - totalSend, 0);
			totalSend += numBytesSend;
		} while (numBytesSend != -1 && totalSend < length);

		if (numBytesSend == -1) {
			clientsToRemove.push_back(g_clients[i]);
		}

	}

	//Hacer mas limpio
	for (size_t i = 0; i < clientsToRemove.size(); i++)
	{
		SOCKET* clientToRemove = clientsToRemove[i];
		for (auto itClients = g_clients.begin(); itClients != g_clients.end(); ++itClients) {
			if (*itClients == clientToRemove) {
				g_clients.erase(itClients);
				break;
			}
		}
	}
	pthread_mutex_unlock(&g_mutex);
}