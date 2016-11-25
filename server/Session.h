#pragma once
#include <string>
#include <WinSock2.h>

using namespace std;


inline string GetSessionName(struct sockaddr_in sin)
{
	// ReSharper disable once CppDeprecatedEntity
	return string(string(inet_ntoa(sin.sin_addr)) += ":")
		+= to_string(sin.sin_port);
}

inline string GetSessionName(SOCKET socket)
{
	struct sockaddr_in sin;
	int len = sizeof(sin);
	if (getsockname(socket, reinterpret_cast<struct sockaddr *>(&sin), &len) == -1)
		throw - 3;
	return GetSessionName(sin);
}

class Session
{
public:
	SOCKET ClientSocket = NULL;
	string Name;
	struct sockaddr_in Sin;
	string LastCommand;
	string FilePath;
	int FileSize;
	int LastPosition;
	bool IsSuccess;
	bool isUdp;

public:
	explicit Session(SOCKET socket)
	{
		ClientSocket = socket;
		Name = GetSessionName(socket);
		clearSessionData();
		isUdp = false;
	}
	explicit Session(struct sockaddr_in sin)
	{
		Sin = sin;
		ClientSocket = NULL;
		Name = GetSessionName(sin);
		clearSessionData();
		isUdp = true;
	}
	void clearSessionData()
	{
		FilePath = "";
		FileSize = LastPosition = 0;
		IsSuccess = true;
		LastCommand = "";
	}
	void setSessionData(string command, string filePath, int fileSize, int lastPosition)
	{
		FilePath = filePath;
		FileSize = fileSize;
		LastPosition = lastPosition;
		createLastCommand(command);
		IsSuccess = false;
	}
	void setSocket(SOCKET socket)
	{
		Sin = GetSin(socket);
		ClientSocket = socket;
	}
private:
	void createLastCommand(string command)
	{
		LastCommand += command += " ";
		LastCommand += FilePath += " ";
		LastCommand += to_string(FileSize) + " ";
		LastCommand += to_string(LastPosition);
	}
	static struct sockaddr_in GetSin(SOCKET socket)
	{
		struct sockaddr_in sin;
		int len = sizeof(sin);
		if (getsockname(socket, reinterpret_cast<struct sockaddr *>(&sin), &len) == -1)
			throw - 3;
		return sin;
	}
};
