#pragma once
#include <string>
#include <WinSock2.h>

using namespace std;

class Session
{
public:
	SOCKET ClientSocket = NULL;
	string Name;
	string Ip;
	int Port;
	string LastCommand;
	string FilePath;
	int FileSize;
	int LastPosition;
	bool IsSuccess;

public:
	explicit Session(SOCKET socket, string ip, int port)
	{
		ClientSocket = socket;
		Ip = ip;
		Name = ip += string(":") += to_string(port);
		Port = port;
		clearSessionData();
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
private:
	void createLastCommand(string command)
	{
		LastCommand += command += " ";
		LastCommand += FilePath += " ";
		LastCommand += to_string(FileSize) + " ";
		LastCommand += to_string(LastPosition);
	}
};