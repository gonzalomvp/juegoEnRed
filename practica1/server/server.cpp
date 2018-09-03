// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#define SERVER_PORT 12345
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

std::vector<SOCKET*> g_clients;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void* receiveMessages(void* argument);
void sendMessageToClients(const char* message);

int main(int argc, char *argv[])
{
	bool error = false;
	WSADATA wsaData;
	int wsaerr;
	SOCKET sockId;
	int on = 1;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0)
	{
		printf("Could not initialize Winsock.\n");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // Aquí NO se usa htons.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	sockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockId < 0)
	{
		printf("Error creating socket\n");
		error = true;
	}
	else
	{
		setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	}
	
	if (!error && bind(sockId, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		printf("Error binding port\n");
		error = true;
	}
	if (!error && listen(sockId, QUEUE_SIZE) < 0)
	{
		printf("listen failed\n");
		error = true;
	}
	if (error)
	{
		WSACleanup();
		return -1;
	}

	//keep accepting client connections
	while (1)
	{
		socklen_t sock_len = sizeof(client);
		memset(&client, 0, sizeof(client));
		SOCKET socketCliId = accept(sockId, (struct sockaddr *)&client, &sock_len);
		if (socketCliId == INVALID_SOCKET)
			printf("Can't accept client:%d\n", WSAGetLastError());
		
		//create a thread to handle messages received from the client
		else
		{
			SOCKET* socketInThread = new SOCKET(socketCliId);

			pthread_mutex_lock(&g_mutex);
			g_clients.push_back(socketInThread);
			pthread_mutex_unlock(&g_mutex);

			pthread_t receiveMessagesThread;
			pthread_create(&receiveMessagesThread, NULL, receiveMessages, socketInThread);
		}
	}

	//close server socket
	closesocket(sockId);

	//close all client sockets from list and release resources
	pthread_mutex_lock(&g_mutex);
	for (size_t i = 0; i < g_clients.size(); ++i)
	{
		closesocket(*g_clients[i]);
		delete g_clients[i];
	}
	g_clients.clear();
	pthread_mutex_unlock(&g_mutex);

	WSACleanup();
    return 0;
}

void* receiveMessages(void* argument)
{
	bool connectionError = false;
	char buf[BUF_SIZE];
	char messageToSend[BUF_SIZE];
	char nick[BUF_SIZE];
	int rec = 0;
	int totalRecv = 0;

	SOCKET* socketCliId = (SOCKET*)argument;

	//receive nick
	totalRecv = 0;
	do
	{
		rec = recv(*socketCliId, buf + totalRecv, BUF_SIZE, 0);
		totalRecv += rec;
	} while (rec != -1 && buf[totalRecv - 1] != '\0');

	if (rec != -1)
	{
		strcpy_s(nick, buf);
		printf("%s is now connected\n", nick);
		strcpy_s(messageToSend, nick);
		strcat_s(messageToSend, " is now connected");
		sendMessageToClients(messageToSend);
	}
	else
	{
		connectionError = true;
	}

	//keep receiving from client
	while (!connectionError)
	{
		totalRecv = 0;
		do
		{
			rec = recv(*socketCliId, buf + totalRecv, BUF_SIZE, 0);
			totalRecv += rec;
		} while (rec != -1 && buf[totalRecv - 1] != '\0');

		//message to resend to all clients
		if (rec != -1)
		{
			strcpy_s(messageToSend, nick);
			strcat_s(messageToSend, "> ");
			strcat_s(messageToSend, buf);
		}
		//disconnection message
		else
		{
			connectionError = true;
			strcpy_s(messageToSend, nick);
			strcat_s(messageToSend, " has been disconnected");
		}
		printf("%s\n", messageToSend);
		sendMessageToClients(messageToSend);
	}

	//remove socket from list
	pthread_mutex_lock(&g_mutex);
	auto it = std::find(g_clients.begin(), g_clients.end(), socketCliId);
	if (it != g_clients.end()) g_clients.erase(it);
	pthread_mutex_unlock(&g_mutex);

	//close socket and release resources
	closesocket(*socketCliId);
	delete socketCliId;

	return nullptr;
}

void sendMessageToClients(const char* message)
{
	int length = strlen(message) + 1;

	pthread_mutex_lock(&g_mutex);
	for (size_t i = 0; i < g_clients.size(); ++i)
	{
		int numBytesSend, totalSend = 0;
		do
		{
			numBytesSend = send(*g_clients[i], message, length - totalSend, 0);
			totalSend += numBytesSend;
		} while (numBytesSend != -1 && totalSend < length);
	}
	pthread_mutex_unlock(&g_mutex);
}