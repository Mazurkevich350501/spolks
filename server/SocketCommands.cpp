#include "SocketCommands.h"
#include <iostream>
#include <string>

#define messageBufferLength 1024
#define packageLength 40960

void ShowMessage(string message)
{
	cout << message;
}

int FileSize(string filePath)
{
	std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
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

int sendOOBMessage(SOCKET socket, char message)
{
	int ret = send(socket, &message, 1, MSG_OOB);
	return ret;
}

int SendFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession)
{ 
	if (currentSession != nullptr)
		currentSession->setSessionData("startDownload", filePath, fileSize, startPosition);

	fstream file;
	file.open(filePath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) return -1;
	
	int nSendSize = file.tellg();
	if (fileSize != nSendSize) return -2;
	
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

void ShowDownloadSize(SOCKET socket, int downloadSize)
{
	char buf[2];
	if (recv(socket, buf, 1, MSG_OOB) != SOCKET_ERROR)
	{
		ShowMessage(string("download: ") += to_string(downloadSize) += "\n");
	}
}

int ReadPackege(SOCKET socket, char* package)
{
	return recv(socket, package, packageLength, 0);
}

int ReadFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession)
{
	if (currentSession != nullptr)
		currentSession->setSessionData("startUpload", filePath, fileSize, startPosition);
	
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
		if (currentSession != nullptr)
			currentSession->LastPosition = downloadSize;
	}
	file.close();
	if (currentSession != nullptr)
		currentSession->clearSessionData();
	return 1;
}