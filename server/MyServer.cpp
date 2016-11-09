#include "MyServer.h"
#include "SocketCommands.h"
#include "CommandParser.h"
#include <iostream>

bool MyServer::StartServer(char* addres, int host, int clientCount) {
	SOCKET	sServerListen;
	struct sockaddr_in localaddr;

	sServerListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sServerListen == SOCKET_ERROR)
	{
		throw -3;
	}
	// ReSharper disable once CppDeprecatedEntity
	localaddr.sin_addr.s_addr = inet_addr(addres);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(host);

	if (bind(sServerListen, reinterpret_cast<struct sockaddr *>(&localaddr), sizeof(localaddr)) == SOCKET_ERROR)
	{
		throw -4;
	}

	cout << "Bind OK" << endl;
	listen(sServerListen, clientCount);
	cout << "Listen OK" << endl;

	ServerSocket = sServerListen;
	return true;
}

bool MyServer::ClientAccept()
{
	SOCKET sClient;
	struct sockaddr_in clientaddr;
	int iSize = sizeof(clientaddr);

	sClient = accept(ServerSocket, reinterpret_cast<struct sockaddr *>(&clientaddr), &iSize);
	if (sClient == INVALID_SOCKET)
	{
		throw sClient;
	}
	ClientSocket = sClient;
	AddSession(ClientSocket);
	ShowMessage(string(string("Connect client ") += ActiveSessionName) += "\n");
	return true;
}

int MyServer::ExecuteClientThread()
{
	Session* session = GetSession();
	string message = ">";
	if(!session->IsSuccess)
	{
		message = session->LastCommand;
	}
	SendSocketMessage(ClientSocket, message);
	while(true)
	{
		message = ReadSocketMessage(ClientSocket);
		int result = Execute(message);
		if (result < 0) return result;
	}
}

void MyServer::CloseSocket()
{
	RemoveSession();
	closesocket(ClientSocket);
}

void MyServer::AddSession(SOCKET socket)
{	
	struct sockaddr_in sin;
	int len = sizeof(sin);
	if (getsockname(socket, reinterpret_cast<struct sockaddr *>(&sin), &len) == -1)
		throw -3;
	// ReSharper disable once CppDeprecatedEntity
	string name = string(string(inet_ntoa(sin.sin_addr)) += ":")
		+= to_string(sin.sin_port);
	if(GetSession(name) == nullptr)
	{
		ActiveSessionName = name;
		// ReSharper disable once CppDeprecatedEntity
		Session newSession(inet_ntoa(sin.sin_addr), sin.sin_port);
		Sessions.push_back(newSession);
	}
}

void MyServer::RemoveSession()
{
	if (GetSession(ActiveSessionName)->IsSuccess)
	{
		Session findSession(ActiveSessionName);
		int index = GetSessionIndex(ActiveSessionName);
		if (index != -1)
		{
			Sessions.erase(Sessions.begin() + index);
			ShowMessage(string(string("Disconnect client ") += ActiveSessionName)  += "\n");
		}
		ActiveSessionName = "";
	}
}

Session* MyServer::GetSession(string name)
{
	if (name == "") name = ActiveSessionName;
	for (int i = Sessions.size() - 1; i >= 0; i--)
	{
		if (Sessions[i].Name == name)
		{
			return &Sessions[i];
		}
	}
	return nullptr;
}

int MyServer::GetSessionIndex(string name)
{
	for (int i = Sessions.capacity() - 1; i >= 0; i++)
	{
		if (Sessions[i].Name == name)
		{
			return i;
		}
	}
	return -1;
}
