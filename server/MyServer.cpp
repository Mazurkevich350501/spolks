#include "MyServer.h"
#include "SocketCommands.h"
#include "CommandParser.h"
#include <iostream>


bool MyServer::StartServer(char* addres, int host, int clientCount) {
	SOCKET	sServerListen;
	SOCKET	sServerListenUDP;
	struct sockaddr_in localaddr;
	
	sServerListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sServerListenUDP = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
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
	ServerSocketUDP = sServerListenUDP;
	return true;
}

void MyServer::ConnectCommand(Session session)
{
	string message = session.IsSuccess
		? ">"
		: session.LastCommand;
	if(session.IsSuccess)
	{
		SendSocketMessage(session.ClientSocket, message);
	}
	else
	{
		Execute(message, session.ClientSocket);
	}
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
	Session session = AddSession(sClient);
	ShowMessage(string(string("Connect client ") += session.Name) += "\n");
	ConnectCommand(session);
	
	return true;
}

int MyServer::ServerProcess()
{
	while(true)
	{
		InitSets();
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 30;
		int activity = select(max_sd + 1, &Readfds, NULL, NULL, &tv);
		if ((activity < 0) && (errno != EINTR))
		{
			ShowMessage("select error\n");
		}
		else
		{
			if (FD_ISSET(ServerSocket, &Readfds))
			{
				ClientAccept();
				continue;
			}
			ExecuteClientRequest();
		}
	}
}

void MyServer::CloseSocket(Session session)
{
	RemoveSession(session);
}

Session MyServer::AddSession(SOCKET socket)
{	
	struct sockaddr_in sin;
	int len = sizeof(sin);
	if (getsockname(socket, reinterpret_cast<struct sockaddr *>(&sin), &len) == -1)
		throw -3;
	// ReSharper disable once CppDeprecatedEntity
	string name = string(string(inet_ntoa(sin.sin_addr)) += ":")
		+= to_string(sin.sin_port);
	auto session = GetSession(name);
	if(session == nullptr)
	{
		// ReSharper disable once CppDeprecatedEntity
		Session newSession(socket, inet_ntoa(sin.sin_addr), sin.sin_port);
		Sessions.push_back(newSession);
		return newSession;
	}
	session->ClientSocket = socket;
	return GetSession(name)[0];
}

void MyServer::RemoveSession(Session session)
{
	int index = GetSessionIndex(session.Name);
	if (index != -1)
	{
		if(session.IsSuccess)
		{
			Sessions.erase(Sessions.begin() + index);
			ShowMessage(string(string("Disconnect client ") += session.Name) += "\n");
		}
		else
		{
			GetSession(session.Name)->ClientSocket = 0;
			ShowMessage(string(string("Disconnect client ") += session.Name) += "\n");
		}
	}
}

Session* MyServer::GetSession(string name)
{
	for (int i = Sessions.size() - 1; i >= 0; i--)
	{
		if (Sessions[i].Name == name)
		{
			return &Sessions[i];
		}
	}
	return nullptr;
}

Session* MyServer::GetSession(SOCKET socket)
{
	for (int i = Sessions.size() - 1; i >= 0; i--)
	{
		if (Sessions[i].ClientSocket == socket)
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

bool MyServer::InitSets()
{
	FD_ZERO(&Readfds);
	FD_ZERO(&Writefds);
	
	FD_SET(ServerSocket, &Readfds);
	max_sd = ServerSocket;
	/*FD_SET(ServerSocketUDP, &Readfds);
	if (ServerSocketUDP > max_sd)
		max_sd = ServerSocketUDP;*/

	for(int i = 0; i < Sessions.size(); i++)
	{
		SOCKET socket = Sessions[i].ClientSocket;
		if(socket > 0)
		{
			auto set = !Sessions[i].IsSuccess && Sessions[i].LastCommand.find("download") != -1
				? &Writefds
				: &Readfds;
			FD_SET(socket, set);
		}
		if (socket > max_sd)
			max_sd = socket;
	}
	return true;
}

int MyServer::ExecuteClientRequest()
{
	string message;
	for (int i = 0; i < Sessions.size(); i++)
	{
		try
		{
			if (FD_ISSET(Sessions[i].ClientSocket, &Readfds))
			{
				if (Sessions[i].IsSuccess)
				{
					message = ReadSocketMessage(Sessions[i].ClientSocket);
					Execute(message, Sessions[i].ClientSocket);
				}
				else
				{
					ReadSocketPackege(Sessions[i]);
				}
			}
			else if (FD_ISSET(Sessions[i].ClientSocket, &Writefds))
			{
				SendSocketPackege(Sessions[i]);
			}
		}
		catch (int ex)
		{
			CloseSocket(Sessions[i]);
			if (i < Sessions.size())
				if (Sessions[i].ClientSocket != 0)
				{
					i--;
				}
		}
	}
	return 1;
}
