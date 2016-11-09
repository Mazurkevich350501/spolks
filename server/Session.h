#pragma once
#include <string>

using namespace std;

class Session
{
public:
	string Name;
	string Ip;
	int Port;
	string LastCommand;
	string Filepath;
	int FileSize;
	int LastPosition;
	bool IsSuccess;

public:
	explicit Session(string ip, int port = 0)
	{
		Ip = ip;
		Name = ip += string(":") += to_string(port);
		Port = port;
		clearSessionData();
	}
	void clearSessionData()
	{
		Filepath = "";
		FileSize = LastPosition = 0;
		IsSuccess = true;
	}
	void setSessionData(string command, string filePath, int fileSize, int lastPosition)
	{

		Filepath = filePath;
		FileSize = fileSize;
		LastPosition = lastPosition;
		createLastCommand(command);
		IsSuccess = false;
	}
private:
	void createLastCommand(string command)
	{
		LastCommand += command += " ";
		LastCommand += Filepath += " ";
		LastCommand += to_string(FileSize) + " ";
		LastCommand += to_string(LastPosition);
	}
};