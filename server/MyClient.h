#pragma once

#include <string>
#include <WinSock2.h>

using namespace std;

class MyClient
{
private:
	struct sockaddr_in server;
	SOCKET Socket = NULL;
	char serverAddres[17];
	int port;

public:
	MyClient(): port(0)
	{
	}

	void StartCient(string addres, int host);
	bool Connect();
	void Disconnect() const;
	static bool Reconnect();
	int Execute();
	~MyClient()
	{
		closesocket(Socket);
	}
private:
	int ExecuteCommand(string message);
};