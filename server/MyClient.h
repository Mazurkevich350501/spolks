#pragma once

#include <string>
#include <WinSock2.h>

using namespace std;

class MyClient
{
public:
	sockaddr_in ServerSin;
	SOCKET Socket = NULL;
	bool IsUdp = false;

	void StartCient(string addres, int host);
	bool Connect();
	void Disconnect() const;
	bool Reconnect();
	int Execute() const;
	~MyClient()
	{
		closesocket(Socket);
	}
private:
	int ExecuteCommand(string message) const;
};