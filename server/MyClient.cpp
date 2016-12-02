#include "MyClient.h"
#include "SocketCommands.h"
#include <iostream>
#include <ctime>

void MyClient::StartCient(string serverIp, int serverPort)
{
	char serverAddres[20];
	// ReSharper disable once CppDeprecatedEntity
	strcpy(serverAddres, serverIp.c_str());
	int port = serverPort;
	struct hostent	*host;

	ServerSin.sin_family = AF_INET;
	ServerSin.sin_port = htons(port);
	// ReSharper disable once CppDeprecatedEntity
	ServerSin.sin_addr.s_addr = inet_addr(serverAddres);

	if (ServerSin.sin_addr.s_addr == INADDR_NONE)
	{
		// ReSharper disable once CppDeprecatedEntity
		host = gethostbyname(serverAddres);
		if (host == nullptr)
		{
			ShowMessage("Unable to resolve server");
			return;
		}
		CopyMemory(&ServerSin.sin_addr, host->h_addr_list[0],
			host->h_length);
	}
}

SOCKET CreateSocket(bool isTcp)
{
	SOCKET newSocket = isTcp
		? socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
		: socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newSocket == INVALID_SOCKET)
	{
		ShowMessage("Can't create socket");
		throw static_cast<int>(newSocket);
	}
	return newSocket;
}

int GetChoice() 
{
	static int result = 0;
	if (result == 0)
	{
		while (result < 1 || result > 2)
		{
			cout << "Choice protocol:\n" << "1-tcp, 2-udp: ";
			cin >> result;
			cin.ignore(INT_MAX, '\n');
		}
	}
	return result;
}

bool MyClient::Connect()
{	
	int choice = GetChoice();
	IsUdp = choice == 2;
	Socket = CreateSocket(!IsUdp);
	if (!IsUdp)
	{
		if (connect(Socket, reinterpret_cast<struct sockaddr *>(&ServerSin),
			sizeof(ServerSin)) == SOCKET_ERROR)
		{
			return false;
		}
	}
	else
	{
		try
		{
			SendSocketMessage(Socket, ServerSin, "connect\r\n");
			ReadSocketMessage(Socket, ServerSin);
		}
		catch (int)
		{
			return false;
		}
		SendSocketMessage(Socket, ServerSin, "connect\r\n");
	}
	return true;
}

void MyClient::Disconnect() const
{
	closesocket(Socket);
}

void TimeSpan(int ms)
{
	const clock_t begin_time = clock();
	while ((clock() - begin_time) < ms);
}

bool MyClient::Reconnect()
{
	const clock_t begin_time = clock();
	do
	{
		if (Connect())
			return true;
		TimeSpan(30);
	}
	while((clock() - begin_time) / CLOCKS_PER_SEC < 10);
	return false;
}

int MyClient::Execute() const
{
	string message;
	message = ReadSocketMessage(Socket, ServerSin);
	return  ExecuteCommand(message);
}
