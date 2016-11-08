#include "MyClient.h"
#include "SocketCommands.h"
#include "CommandParser.h"
#include <map>
#include <iostream>


int Upload(SOCKET socket, CommandParser params);
int Download(SOCKET socket, CommandParser params);
int CloseClient(SOCKET socket, int CommandCount, string message);
int TypeCommand(SOCKET socket);


enum sendCommands 
{  
	startUpload = 1,
	startDownload,
	close,
	error,
	success,
	typeCommand
};

map<string, sendCommands> SendCommandMapping()
{
	map<string, sendCommands> mapping;

	mapping["startUpload"] = startUpload;
	mapping["startDownload"] = startDownload;
	mapping["close"] = close;
	mapping["error"] = error;
	mapping["success"] = success;
	mapping[">"] = typeCommand;
	return mapping;
}

int MyClient::ExecuteCommand(string message)
{
	CommandParser commandParcer;
	commandParcer.setMessage(message);
	string command = commandParcer.getCommand();
	map<string, sendCommands> mapping = SendCommandMapping();

	switch (mapping[command])
	{
	case startUpload:
		return Upload(Socket, commandParcer);
	case startDownload:
		return Download(Socket, commandParcer);
	case close:
		return CloseClient(Socket, commandParcer.getParamsCount(), message);
	case error:
	case success:
		ShowMessage(message);
	case typeCommand:
		return TypeCommand(Socket);
	default:
		ShowMessage(message);
		return TypeCommand(Socket);
	}
}

int Upload(SOCKET socket, CommandParser params)
{
	string filePath = params.getParam(1);
	int fileSize = FileSize(filePath);

	if (fileSize)
	{
		SendSocketMessage(socket, "success");
		int result = SendFile(socket, filePath, stoi(params.getParam(2)), stoi(params.getParam(3)), NULL);
		return result;
	}
	else
	{
		SendSocketMessage(socket, "error");
		return 0;
	}
}

int Download(SOCKET socket, CommandParser params)
{
	int result = ReadFile(socket, params.getParam(1), stoi(params.getParam(2)), stoi(params.getParam(3)), NULL);
	SendSocketMessage(socket, result > 0 ? "success" : "error");
	return result;
}

int CloseClient(SOCKET socket, int CommandCount, string message)
{
	if (CommandCount > 0) return -1;
	ShowMessage(message);
	return TypeCommand(socket);
}

string AddingMessage(string message)
{
	CommandParser parser;
	parser.setMessage(message);
	if (parser.getCommand() == "upload")
	{
		int fileSize = FileSize(parser.getParam(1));
		if (fileSize)
		{
			message += " ";
			return message += to_string(fileSize);
		}
		return "error";
	}
	return message;
}

int TypeCommand(SOCKET socket)
{
	ShowMessage(">");
	string message;
	getline(cin, message);
	SendSocketMessage(socket, AddingMessage(message));
	return 1;
}