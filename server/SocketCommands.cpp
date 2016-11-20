#include "SocketCommands.h"
#include <iostream>
#include <string>
#include <fstream>

#define messageBufferLength 1024
#define packageLength 200000

void ShowMessage(string message)
{
	cout << message;
}

int FileSize(string filePath)
{
	ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
	int result = in.tellg();
	return result < 0 ? 0 : result;
}

string ReadSocketMessage(SOCKET socket)
{
	string message = "";
	char recvBuffer[messageBufferLength];
	while (message.find("\r\n") == -1)
	{
		int ret = recv(socket, recvBuffer, messageBufferLength, 0);
		if (ret == SOCKET_ERROR)
		{
			throw ret;
		}
		recvBuffer[ret] = '\0';
		message.append(recvBuffer);
	}
	return message;
}

int SendSocketMessage(SOCKET socket, string message)
{
	message.append("\r\n");
	char sendMessage[messageBufferLength];
	// ReSharper disable once CppDeprecatedEntity
	strcpy(sendMessage, message.c_str());
	int ret = send(socket, sendMessage, strlen(sendMessage), 0);
	if (ret == SOCKET_ERROR)
		throw ret;
	return ret;
}

int SendPackage(SOCKET socket, char* package, int size)
{
	int ret = send(socket, package, size, 0);
	return ret;
}

int SendSocketPackege(Session &session)
{
	fstream file;
	file.open(session.FilePath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) return -1;

	file.seekg(session.LastPosition, ios_base::beg);
	char package[packageLength];
	if (session.LastPosition < session.FileSize)
	{
		int sendSize = session.FileSize - session.LastPosition < packageLength
			? session.FileSize - session.LastPosition
			: packageLength;
		int ret = SendPackage(session.ClientSocket, package, sendSize);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		file.write(package, ret);
		session.LastPosition += ret;
		file.close();
	}
	if(session.LastPosition == session.FileSize)
	{
		session.clearSessionData();
		string message = ReadSocketMessage(session.ClientSocket);
		SendSocketMessage(session.ClientSocket, message);
		return message == "success\r\n" ? 1 : 0;
	}
	return 1;
}

int SendFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession)
{ 
	if (currentSession != nullptr)
		currentSession->setSessionData("download", filePath, fileSize, startPosition);

	fstream file;
	file.open(filePath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) return -1;
	
	int nSendSize = file.tellg();
	
	nSendSize -= startPosition;
	file.seekg(startPosition, ios_base::beg);
	char package[packageLength];

	while (nSendSize > 0)
	{
		int sendSize = nSendSize < packageLength ? nSendSize : packageLength;
		file.read(package, packageLength);
		int ret = SendPackage(socket, package, sendSize);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		nSendSize -= ret;
		if (currentSession != nullptr)
			currentSession->LastPosition += ret;
	}
	file.close();
	if (currentSession != nullptr)
		currentSession->clearSessionData();
	return 1;
}

int ReadPackege(SOCKET socket, char* package)
{
	return recv(socket, package, packageLength, 0);
}

int ReadSocketPackege(Session &session)
{
	fstream file;
	file.open(session.FilePath, ios::out | ios::binary | ios::ate);
	if (!file.is_open()) return -1;
	
	char package[packageLength];
	file.seekg(session.LastPosition, ios_base::end);
	if(session.LastPosition < session.FileSize)
	{
		int ret = ReadPackege(session.ClientSocket, package);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		file.write(package, ret);
		session.LastPosition += ret;
		file.close();
		ShowMessage(string(to_string(session.LastPosition)) += string(" : ")
			+= to_string(session.FileSize) += "\n");
	}
	if(session.LastPosition == session.FileSize)
	{
		session.clearSessionData();
		SendSocketMessage(session.ClientSocket, "success");
	}
	return 1;
}

int ReadFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession)
{
	if (currentSession != nullptr)
		currentSession->setSessionData("upload", filePath, fileSize, startPosition);
	
	fstream file;
	file.open(filePath, ios::out | ios::binary | ios::ate);
	if (!file.is_open()) return -1;

	char package[packageLength];
	int downloadSize = startPosition;
	file.seekg(startPosition, ios_base::end);
	while (downloadSize < fileSize)
	{
		int ret = ReadPackege(socket, package);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		file.write(package, ret);
		downloadSize += ret;
		ShowMessage(string(to_string(downloadSize)) += string(" : ") 
			+= to_string(fileSize) += "\n");
		if (currentSession != nullptr)
			currentSession->LastPosition = downloadSize;
	}
	file.close();
	if (currentSession != nullptr)
		currentSession->clearSessionData();
	return 1;
}