#include "MyServer.h"
#include "SocketCommands.h"
#include "CommandParser.h"
#include <iostream>


bool MyServer::StartServer(char* addres, int hostTcp, int clientCount) {
	SOCKET sServerListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET sServerListenUdp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct sockaddr_in localaddr;
	
	if (sServerListen == SOCKET_ERROR || sServerListenUdp == SOCKET_ERROR)
	{
		throw -3;
	}
	// ReSharper disable once CppDeprecatedEntity
	localaddr.sin_addr.s_addr = inet_addr(addres);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(hostTcp);

	if (bind(sServerListenUdp, reinterpret_cast<struct sockaddr *>(&localaddr), sizeof(localaddr)) == SOCKET_ERROR
		|| bind(sServerListen, reinterpret_cast<struct sockaddr *>(&localaddr), sizeof(localaddr)) == SOCKET_ERROR)
	{
		throw -4;
	}

	cout << "Bind OK" << endl;
	listen(sServerListen, clientCount);
	cout << "Listen OK" << endl;

	ServerSocketTcp = sServerListen;
	ServerSocketUdp = sServerListenUdp;
	return true;
}

void MyServer::ConnectCommand(Session* session)
{
	string message = session->IsSuccess
		? ">"
		: session->LastCommand;
	if(session->IsSuccess)
	{
		SendSocketMessage(session->ClientSocket, session->Sin, message);
	}
	else
	{
		Execute(message, session->ClientSocket, session);
	}
}

bool MyServer::ClientAccept()
{
	SOCKET sClient;
	struct sockaddr_in clientaddr;
	int iSize = sizeof(clientaddr);

	sClient = accept(ServerSocketTcp, reinterpret_cast<struct sockaddr *>(&clientaddr), &iSize);
	if (sClient == INVALID_SOCKET)
	{
		throw sClient;
	}
	Session* session = AddSession(sClient);
	ShowMessage(string(string("Connect client ") += session->Name) += "\n");
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
		tv.tv_usec = 5;
		int activity = select(max_sd + 1, &Readfds, NULL, NULL, &tv);
		if ((activity < 0) && (errno != EINTR))
		{
			ShowMessage("select error\n");
		}
		else
		{
			if (FD_ISSET(ServerSocketTcp, &Readfds))
			{
				ClientAccept();
				continue;
			}
			if (FD_ISSET(ServerSocketUdp, &Readfds))
			{
				ExecuteUdpClientRequest();
				continue;
			}
			ExecuteTcpClientRequest();
		}
	}
}

void MyServer::CloseSocket(Session session)
{
	RemoveSession(session);
}

Session* MyServer::AddSession(SOCKET socket)
{	
	string name = GetSessionName(socket);
	auto session = GetSession(name);
	if(session == nullptr)
	{
		// ReSharper disable once CppDeprecatedEntity
		Session newSession(socket);
		Sessions.push_back(newSession);
		return GetSession(name);
	}
	session->setSocket(socket);
	return session;
}

Session* MyServer::AddSession(sockaddr_in sin)
{
	string name = GetSessionName(sin);
	auto session = GetSession(name);
	if (session == nullptr)
	{
		// ReSharper disable once CppDeprecatedEntity
		Session newSession(sin);
		Sessions.push_back(newSession);
		session = GetSession(name);
		session->ClientSocket = ServerSocketUdp;
		return session;
	}
	session->Sin = sin;
	return session;
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
	
	FD_SET(ServerSocketTcp, &Readfds);
	max_sd = ServerSocketTcp;
	FD_SET(ServerSocketUdp, &Readfds);
	if (ServerSocketUdp > max_sd)
		max_sd = ServerSocketUdp;

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

int MyServer::ExecuteTcpClientRequest()
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
					message = ReadSocketMessage(Sessions[i].ClientSocket, Sessions[i].Sin);
					Execute(message, Sessions[i].ClientSocket, GetSession(Sessions[i].Name));
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
		catch (int)
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

int MyServer::ExecuteUdpClientRequest()
{
	string message;
	sockaddr_in sin;
	int sinLength = sizeof(sin);
	int ret = recvfrom(ServerSocketUdp, NULL, 0, MSG_PEEK,
		reinterpret_cast<sockaddr *>(&sin), &sinLength);
	Session* session = AddSession(sin);
	try
	{		
		if (session->IsSuccess)
		{
			message = ReadSocketMessage(ServerSocketUdp, session->Sin);
			Execute(message, ServerSocketUdp, session);
		}
	}
	catch (int)
	{
		CloseSocket(session[0]);
	}
	return 1;
}
