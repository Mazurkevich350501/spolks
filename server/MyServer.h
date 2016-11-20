#pragma once

#include <WinSock2.h>
#include <vector>
#include "Session.h"

using namespace std;

class MyServer 
{
private:
	SOCKET ServerSocket = NULL;
	SOCKET ServerSocketUDP = NULL;
	vector<Session> Sessions;
	fd_set Readfds;
	fd_set Writefds;
	SOCKET max_sd = 0;

public:
	bool StartServer(char* addres, int host, int clientCount);
	bool ClientAccept();
	int ServerProcess();
	void CloseSocket(Session session);
	~MyServer() 
	{
		for(int i = 0; i < Sessions.size(); i++)
		{
			closesocket(Sessions[i].ClientSocket);
		}
		closesocket(ServerSocket);
	}
private:
	Session* GetSession(string name = "");
	Session * GetSession(SOCKET socket);
	int Execute(string message, SOCKET socket);
	Session AddSession(SOCKET socket);
	void RemoveSession(Session session);
	int GetSessionIndex(string name);
	bool InitSets();
	int ExecuteClientRequest();
	void ConnectCommand(Session session);
};