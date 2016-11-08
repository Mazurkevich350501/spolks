#pragma once

using namespace std;

class Session
{
public:
	string Name;
	string Ip;
	int Port;
	string Filepath;
	int FileSize;
	int LastPosition;
	bool IsSuccess;

public:
	explicit Session(string name, int port = 0)
	{
		Name = name;
		Port = port;
		clearSessionData();
	}
	void clearSessionData()
	{
		Filepath = "";
		FileSize = LastPosition = 0;
		IsSuccess = true;
	}
	void setSessionData(string filePath, int fileSize, int lastPosition)
	{
		Filepath = filePath;
		FileSize = fileSize;
		LastPosition = lastPosition;
		IsSuccess = false;
	}
};