#include "SocketCommands.h"
#include <iostream>
#include <string>
#include <fstream>

#define messageBufferLength 1024
#define packageLength 800000

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

string ReadSocketMessage(SOCKET boundSocket, struct sockaddr_in fromAddr)
{
	string message = "";
	char recvBuffer[messageBufferLength];
	int fromLength = sizeof(fromAddr);
	while (message.find("\r\n") == -1)
	{
		int ret = recvfrom(boundSocket, recvBuffer, messageBufferLength, 0,
			reinterpret_cast<sockaddr *>(&fromAddr), &fromLength);
		if (ret == SOCKET_ERROR)
		{
			throw ret;
		}
		recvBuffer[ret] = '\0';
		message.append(recvBuffer);
	}
	return message;
}

int SendSocketMessage(SOCKET boundSocket, struct sockaddr_in toAddr, string message)
{
	message.append("\r\n");
	char sendMessage[messageBufferLength];
	// ReSharper disable once CppDeprecatedEntity
	strcpy(sendMessage, message.c_str());
	int ret = sendto(boundSocket, sendMessage, strlen(sendMessage), 0, 
		reinterpret_cast<sockaddr *>(&toAddr), sizeof(toAddr));
	if (ret == SOCKET_ERROR)
		throw ret;
	return ret;
}

int SendPackage(SOCKET socket, sockaddr_in toAddr, char* package, int size)
{
	return sendto(socket, package, size, 0, 
		reinterpret_cast<sockaddr*>(&toAddr), sizeof(toAddr));
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
		file.read(package, sendSize);
		int ret = SendPackage(session.ClientSocket, session.Sin, package, sendSize);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		session.LastPosition += ret;
		file.close();
	}
	if(session.LastPosition == session.FileSize)
	{
		session.clearSessionData();
		string message = ReadSocketMessage(session.ClientSocket, session.Sin);
		SendSocketMessage(session.ClientSocket, session.Sin, message);
		return message == "success\r\n" ? 1 : 0;
	}
	return 1;
}

int SendFile(SOCKET socket, sockaddr_in toAddr, string filePath, int fileSize, int startPosition)
{ 
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
		int ret = SendPackage(socket, toAddr, package, sendSize);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		nSendSize -= ret;
	}
	file.close();
	return 1;
}

int ReadPackege(SOCKET socket, sockaddr_in fromAddr, char* package)
{
	int fromLen = sizeof(fromAddr);
	return recvfrom(socket, package, packageLength, 0, 
		reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
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
		int ret = ReadPackege(session.ClientSocket, session.Sin, package);
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
		SendSocketMessage(session.ClientSocket, session.Sin, "success");
	}
	return 1;
}

int ReadFile(SOCKET socket, sockaddr_in fromAddr, string filePath, int fileSize, int startPosition)
{
	fstream file;
	file.open(filePath, ios::out | ios::binary | ios::ate);
	if (!file.is_open()) return -1;

	char package[packageLength];
	int downloadSize = startPosition;
	file.seekg(startPosition, ios_base::end);
	while (downloadSize < fileSize)
	{
		int ret = ReadPackege(socket, fromAddr, package);
		if (ret == SOCKET_ERROR)
		{
			file.close();
			throw ret;
		}
		file.write(package, ret);
		downloadSize += ret;
		ShowMessage(string(to_string(downloadSize)) += string(" : ") 
			+= to_string(fileSize) += "\n");
	}
	file.close();
	return 1;
}