
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
#include <WinSock2.h>
#include "MyServer.h"
#include "MyClient.h"

using namespace std;

int ServerMain();
int ClientMain();

bool InitWsaData()
{
	WSADATA WsaData;
	int err = WSAStartup(0x0101, &WsaData);
	if (err == SOCKET_ERROR)
	{
		cout << "WSAStartup() failed: %ld\n" << GetLastError();
		return false;
	}
	return true;
}

int main()
{
	if (!InitWsaData())
	{
		return 1;
	}
	int programMode = 0;
	while (programMode < 1 || programMode > 2)
	{
		cout << "1-server, 2-client: ";
		cin >> programMode;
		cin.ignore(INT_MAX, '\n');
	}
	switch (programMode)
	{
	case 1:
		return ServerMain();
	case 2:
		return ClientMain();
	default: break;
	}
	system("pause");
	return 0;
}

void ShowError(int ex)
{
	cout << "Error: " << ex
		<< "(" << WSAGetLastError() << ")" << endl;
}

int ServerMain()
{
	MyServer server;
	if (server.StartServer("169.254.1.1", 5050, 4))
	{
		while (true)
		{
			try
			{
				server.ClientAccept();
				server.ExecuteClientThread();
			}
			catch (int ex)
			{
				server.CloseSocket();
				ShowError(ex);
			}
		}
	}
	return 0;
}

int ClientMain() 
{
	MyClient client;
	client.StartCient("169.254.113.255", 5050);
	client.Connect();

	while (true)
	{
		try
		{
			int result = client.Execute();
			if (result < 0)
				break;
		}
		catch (int ex)
		{
			ShowError(ex);
			return 0;
		}
	}
	return 1;
}