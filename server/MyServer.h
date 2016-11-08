#pragma once

#include <WinSock2.h>
#include <vector>
#include "Session.h"

using namespace std;

class MyServer 
{
private:
	SOCKET ServerSocket = NULL;
	SOCKET ClientSocket = NULL;
	vector<Session> Sessions;
	string ActiveSessionName;

public:
	bool StartServer(char* addres, int host, int clientCount);
	bool ClientAccept();
	int ExecuteClientThread();
	void CloseSocket();
	~MyServer() 
	{
		closesocket(ClientSocket);
		closesocket(ServerSocket);
	}
private:
	Session* GetSession(string name = "");
	int Execute(string message);
	void AddSession(SOCKET socket);
	void RemoveSession();
	int GetSessionIndex(string name);
};