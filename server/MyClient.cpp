#include "MyClient.h"
#include "SocketCommands.h"
#include <iostream>

void MyClient::StartCient(string serverIp, int serverPort)
{
	// ReSharper disable once CppDeprecatedEntity
	strcpy(serverAddres, serverIp.c_str());
	port = serverPort;
	struct hostent	*host;

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	// ReSharper disable once CppDeprecatedEntity
	server.sin_addr.s_addr = inet_addr(serverAddres);

	if (server.sin_addr.s_addr == INADDR_NONE)
	{
		// ReSharper disable once CppDeprecatedEntity
		host = gethostbyname(serverAddres);
		if (host == nullptr)
		{
			cout << "Unable to resolve server" << endl;
			return;
		}
		CopyMemory(&server.sin_addr, host->h_addr_list[0],
			host->h_length);
	}
}

SOCKET CreateSocket()
{
	SOCKET newSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newSocket == INVALID_SOCKET)
	{
		cout << "Can't create socket" << endl;
	}
	return newSocket;
}

bool MyClient::Connect()
{
	Socket = CreateSocket();
	if (connect(Socket, reinterpret_cast<struct sockaddr *>(&server),
		sizeof(server)) == SOCKET_ERROR)
	{
		cout << "connect failed" << endl;
		return false;
	}

	return true;
}

void MyClient::Disconnect() const
{
	closesocket(Socket);
}

bool MyClient::Reconnect()
{

	return true;
}

int MyClient::Execute()
{
	string message;
	message = ReadSocketMessage(Socket);
	return  ExecuteCommand(message);
}
