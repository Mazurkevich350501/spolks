#pragma once

#include <WinSock2.h>
#include <vector>
#include "Session.h"

using namespace std;

class MyServer 
{
private:
	SOCKET ServerSocketTcp = NULL;
	SOCKET ServerSocketUdp = NULL;
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
		closesocket(ServerSocketTcp);
		closesocket(ServerSocketUdp);
	}
private:
	Session* GetSession(string name = "");
	Session * GetSession(SOCKET socket);
	int Execute(string message, SOCKET socket, Session* session);
	Session* AddSession(SOCKET socket);
	Session* AddSession(sockaddr_in sin);
	void RemoveSession(Session session);
	int GetSessionIndex(string name);
	bool InitSets();
	int ExecuteTcpClientRequest();
	int ExecuteUdpClientRequest(bool isRead);
	void ConnectCommand(Session* session);
};