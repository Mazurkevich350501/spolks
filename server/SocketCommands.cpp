#include "SocketCommands.h"
#include <iostream>
#include <string>
#include <fstream>

#define messageBufferLength 1024

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

void WriteFileToBuffer(Session &session)
{
	fstream file;
	file.open(session.FilePath, ios::in | ios::binary | ios::ate);
	if (!file.is_open()) return;
	int fileSize = file.tellg();
	if (session.LastPosition < fileSize)
	{
		int bufferSize = fileSize - session.LastPosition < session.MaxReadBufferLength
			? fileSize - session.LastPosition
			: session.MaxReadBufferLength;
		file.seekg(session.LastPosition, ios_base::beg);
		file.read(session.ReadBuffer, bufferSize);
		session.ReadBufferLength = bufferSize;
	}
	else
	{
		session.ReadBufferLength = 0;
	}
	session.BufferStartPosition = session.LastPosition;
	file.close();
}

void WriteBufferToFile(Session &session)
{
	fstream file;
	file.open(session.FilePath, ios::out | ios::binary | ios::ate);
	if (!file.is_open()) return;
	file.seekg(session.LastPosition - session.ReadBufferLength, ios_base::end);
	file.write(session.ReadBuffer, session.ReadBufferLength);
	session.BufferStartPosition = session.LastPosition;
	session.ReadBufferLength = 0;
	file.close();
}

int SendPackage(SOCKET socket, sockaddr_in toAddr, char* package, int size)
{
	return sendto(socket, package, size, 0, 
		reinterpret_cast<sockaddr*>(&toAddr), sizeof(toAddr));
}

int SendTcpPackage(Session &session) 
{
	int sendBufferSize = session.LastPosition - session.BufferStartPosition;
	return SendPackage(session.ClientSocket, session.Sin,
		session.ReadBuffer + sendBufferSize, session.ReadBufferLength - sendBufferSize);
}

int SendSocketPackege(Session &session)
{
	if (session.LastPosition < session.FileSize)
	{
		if (session.LastPosition - session.BufferStartPosition == session.ReadBufferLength)
		{
			WriteFileToBuffer(session);
		}
		int ret = SendTcpPackage(session);
		if (ret == SOCKET_ERROR)
		{
			throw ret;
		}
		session.LastPosition += ret;
		cout << ret << endl;
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
	Session session(socket);
	session.Sin = toAddr;
	session.setSessionData("upload", filePath, fileSize, startPosition, false);
	
	while (session.LastPosition != session.FileSize)
	{
		SendSocketPackege(session);
	}
	return 1;
}

int ReadPackege(SOCKET socket, sockaddr_in fromAddr, char* package, int size)
{
	int fromLen = sizeof(fromAddr);
	return recvfrom(socket, package, size, 0,
		reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
}

int ReadTcpPackage(Session &session)
{
	int readBufferSize = session.LastPosition - session.BufferStartPosition;
	return ReadPackege(session.ClientSocket, session.Sin, 
		session.ReadBuffer + readBufferSize, session.MaxReadBufferLength);
}

int ReadSocketPackege(Session &session)
{
	if(session.LastPosition < session.FileSize)
	{
		if (session.ReadBufferLength >= session.MaxReadBufferLength)
		{
			WriteBufferToFile(session);
		}
		int ret = ReadTcpPackage(session);
		if (ret == SOCKET_ERROR)
		{
			throw ret;
		}
		session.LastPosition += ret;
		session.ReadBufferLength += ret;
		ShowMessage(string(to_string(session.LastPosition)) += string(" : ")
			+= to_string(session.FileSize) += "\n");
	}
	if(session.LastPosition == session.FileSize)
	{
		WriteBufferToFile(session);
		session.clearSessionData();
		SendSocketMessage(session.ClientSocket, session.Sin, "success");
	}
	return 1;
}

int ReadFile(SOCKET socket, sockaddr_in fromAddr, string filePath, int fileSize, int startPosition)
{
	Session session(socket);
	session.Sin = fromAddr;
	session.setSessionData("download", filePath, fileSize, startPosition, true);

	while (session.IsDownload())
	{
		ReadSocketPackege(session);
	}
	return 1;
}