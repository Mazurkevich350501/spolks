#include "MyClient.h"
#include "SocketCommands.h"
#include "CommandParser.h"
#include <map>
#include <iostream>


int Upload(const MyClient* client, CommandParser params);
int Download(const MyClient* client, CommandParser params);
int CloseClient(const MyClient* client, int CommandCount, string message);
int TypeCommand(const MyClient* client);


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

int MyClient::ExecuteCommand(string message) const
{
	CommandParser commandParcer;
	commandParcer.setMessage(message);
	string command = commandParcer.getCommand();
	map<string, sendCommands> mapping = SendCommandMapping();

	switch (mapping[command])
	{
	case startUpload:
		return Upload(this, commandParcer);
	case startDownload:
		return Download(this, commandParcer);
	case close:
		return CloseClient(this, commandParcer.getParamsCount(), message);
	case error:
	case success:
		ShowMessage(message);
	case typeCommand:
		return TypeCommand(this);
	default:
		ShowMessage(message);
		return TypeCommand(this);
	}
}

int Upload(const MyClient* client, CommandParser params)
{
	string filePath = params.getParam(1);
	int fileSize = FileSize(filePath);

	if (fileSize != 0 && fileSize > stoi(params.getParam(3)))
	{
		SendSocketMessage(client->Socket, client->ServerSin, "success");
		int result;
		if (client->IsUdp)
		{
			Session session(client->Socket);
			session.Sin = client->ServerSin;

			session.setSessionData("upload", params.getParam(1), stoi(params.getParam(2)), stoi(params.getParam(3)));
			result = Udp::SendFile(session);
		}
		else
		{
			result = SendFile(client->Socket, client->ServerSin, filePath, stoi(params.getParam(2)), stoi(params.getParam(3)));
		}
		return result;
	}
	else
	{
		SendSocketMessage(client->Socket, client->ServerSin, "error");
		return 0;
	}
}

int Download(const MyClient* client, CommandParser params)
{
	int fileSize = FileSize(params.getParam(1));
	bool isDownload = fileSize == stoi(params.getParam(2)) && fileSize != 0;
	SendSocketMessage(client->Socket, client->ServerSin, !isDownload ? to_string(fileSize) : "success");
	if (isDownload)
	{
		if (client->IsUdp)
			SendSocketMessage(client->Socket, client->ServerSin, "connect");
		return 1;
	}

	int result;
	if (client->IsUdp)
	{
		Session session(client->Socket);
		session.Sin = client->ServerSin;
		session.setSessionData("download", params.getParam(1), stoi(params.getParam(2)), fileSize);
		result = Udp::ReadFile(session);
	}
	else
	{
		result = ReadFile(client->Socket, client->ServerSin, params.getParam(1), stoi(params.getParam(2)), fileSize);
	}
	SendSocketMessage(client->Socket, client->ServerSin, result > 0 ? "success" : "error");
	return result;
}

int CloseClient(const MyClient* client, int CommandCount, string message)
{
	if (CommandCount > 0) return -1;
	ShowMessage(message);
	return TypeCommand(client);
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

int TypeCommand(const MyClient* client)
{
	ShowMessage(">");
	string message;
	getline(cin, message);
	SendSocketMessage(client->Socket, client->ServerSin, AddingMessage(message));
	return 1;
}