#include "SocketCommands.h"
#include "CommandParser.h"
#include "MyServer.h"
#include <map>
#include <time.h>

enum commands {getTime = 1, echo, close, upload, download, connectMe, success};

map<string, commands> CommandMapping() {
	map<string, commands> mapping;

	mapping["time"] = getTime;
	mapping["echo"] = echo;
	mapping["close"] = close;
	mapping["upload"] = upload;
	mapping["download"] = download;
	mapping["connect"] = connectMe;
	mapping["success"] = success;

	return mapping;
}

int executeTime(SOCKET socket, Session session);
int executeEcho(SOCKET socket, Session session, string param);
int executeClose(SOCKET socket, Session session);
int executeUpload(SOCKET socket, CommandParser param, Session* currentSession);
int executeDownload(SOCKET socket, CommandParser param, Session* currentSession);


int MyServer::Execute(string message, SOCKET socket, Session* session) 
{	
	CommandParser cParser;
	cParser.setMessage(message);

	string command = cParser.getCommand();
	map<string, commands> mapping = CommandMapping();

	switch (mapping[command]) 
	{
	case getTime:
		return executeTime(socket, session[0]);
	case echo:
		return executeEcho(socket, session[0], cParser.getParam(1));
	case close:
		if(GetSession(socket) != nullptr)
		{
			RemoveSession(session[0]);
		}
		return executeClose(socket, session[0]);
	case upload:
		return executeUpload(socket, cParser, session);
	case download:
		return executeDownload(socket, cParser, session);
	case connectMe:
	case success:
		SendSocketMessage(socket, session[0].Sin, ">");
		return 0;
	default:
		SendSocketMessage(socket, session[0].Sin, "Invalid command");
		return 0;
	}
}

int executeTime(SOCKET socket, Session session)
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	// ReSharper disable once CppDeprecatedEntity
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	string currentTime = buf;

	SendSocketMessage(socket, session.Sin, currentTime);
	return 1;
}

int executeEcho(SOCKET socket, Session session, string param)
{
	SendSocketMessage(socket, session.Sin, param);
	return 1;
}

int executeClose(SOCKET socket, Session session)
{
	SendSocketMessage(socket, session.Sin, "close clientSocket");
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
	string message = createStartTransmitMessage("startUpload", params.getParam(1),
		stoi(params.getParam(2)), startPosition);
	SendSocketMessage(socket, currentSession->Sin, message);
	message = ReadSocketMessage(socket, currentSession->Sin);
	if(message == "success\r\n")
	{
		bool isTcpRead = !currentSession->isUdp;
		currentSession->setSessionData("upload", params.getParam(1), 
			stoi(params.getParam(2)), startPosition, isTcpRead);
	}
	else
	{
		SendSocketMessage(currentSession->ClientSocket, currentSession->Sin, "error");
		return 1;
	}
	SendSocketMessage(currentSession->ClientSocket, currentSession->Sin, "success");
	return 1;
}

int executeDownload(SOCKET socket, CommandParser params, Session* currentSession)
{	
	int fileSize = FileSize(params.getParam(1));
	int startPosition = params.getParam(2) == "" ? 0 : stoi(params.getParam(2));
	if (fileSize)
	{
		string message = createStartTransmitMessage("startDownload", params.getParam(1), 
			fileSize, startPosition);
		SendSocketMessage(socket, currentSession->Sin, message);
		message = ReadSocketMessage(socket, currentSession->Sin);
		if(message == "success\r\n")
		{
			SendSocketMessage(currentSession->ClientSocket, currentSession->Sin, ">");
			return 1;
		}
		startPosition = stoi(message);
	}
	else
	{
		SendSocketMessage(socket, currentSession->Sin, "error");
		return 0;
	}
	currentSession->setSessionData("download", params.getParam(1), fileSize, startPosition, false);
	return 1;
}