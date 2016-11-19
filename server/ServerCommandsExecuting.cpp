#include "SocketCommands.h"
#include "CommandParser.h"
#include "MyServer.h"
#include <map>
#include <time.h>

enum commands {getTime = 1, echo, close, upload, download};

map<string, commands> CommandMapping() {
	map<string, commands> mapping;

	mapping["time"] = getTime;
	mapping["echo"] = echo;
	mapping["close"] = close;
	mapping["upload"] = upload;
	mapping["download"] = download;

	return mapping;
}

int executeTime(SOCKET socket);
int executeEcho(SOCKET socket, string param);
int executeClose(SOCKET socket);
int executeUpload(SOCKET socket, CommandParser param, Session* currentSession);
int executeDownload(SOCKET socket, CommandParser param, Session* currentSession);


int MyServer::Execute(string message) 
{	
	CommandParser cParser;
	cParser.setMessage(message);

	string command = cParser.getCommand();
	map<string, commands> mapping = CommandMapping();

	switch (mapping[command]) 
	{
	case getTime:
		return executeTime(ClientSocket);
	case echo:
		return executeEcho(ClientSocket, cParser.getParam(1));
	case close:
		RemoveSession();
		return executeClose(ClientSocket);
	case upload:
		return executeUpload(ClientSocket, cParser, GetSession());
	case download:
		return executeDownload(ClientSocket, cParser, GetSession());
	default:
		SendSocketMessage(ClientSocket, "Invalid command");
		return 0;
	}
}

int executeTime(SOCKET socket)
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	// ReSharper disable once CppDeprecatedEntity
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	string currentTime = buf;

	SendSocketMessage(socket, currentTime);
	return 1;
}

int executeEcho(SOCKET socket, string param)
{
	SendSocketMessage(socket, param);
	return 1;
}

int executeClose(SOCKET socket)
{
	SendSocketMessage(socket, "close clientSocket");
	return -1;
}

string createStartTransmitMessage(string command, string filePath, int fileSize, int startPosition = 0)
{
	string message = command += " ";
	message += filePath += " ";
	message += to_string(fileSize) + " ";
	message += to_string(startPosition);
	return message;
}

int executeUpload(SOCKET socket, CommandParser params, Session* currentSession)
{
	int startPosition = FileSize(params.getParam(1));
	string message = createStartTransmitMessage("startUpload", params.getParam(1), stoi(params.getParam(2)), startPosition);
	SendSocketMessage(socket, message);
	message = ReadSocketMessage(socket);
	int result = message == "success\r\n"
		? ReadFile(socket, params.getParam(1), stoi(params.getParam(2)), startPosition, currentSession)
		: 0;
	SendSocketMessage(socket, result > 0 ? "success" : "error");
	return result;
}

int executeDownload(SOCKET socket, CommandParser params, Session* currentSession)
{	
	int fileSize = FileSize(params.getParam(1));
	int startPosition = params.getParam(2) == "" ? 0 : stoi(params.getParam(2));
	if (fileSize)
	{
		string message = createStartTransmitMessage("startDownload", params.getParam(1), 
			fileSize, startPosition);
		SendSocketMessage(socket, message);
		message = ReadSocketMessage(socket);
		if(message != "success\r\n")
		{
			startPosition = stoi(message);
		}
	}
	else
	{
		SendSocketMessage(socket, "error");
		return 0;
	}
	
	int result = SendFile(socket, params.getParam(1), fileSize, startPosition, currentSession);
	string message = ReadSocketMessage(socket);
	SendSocketMessage(socket, message);
	return message == "success\r\n" ? result : 0;
}