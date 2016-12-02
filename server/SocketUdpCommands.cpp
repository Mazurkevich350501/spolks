#include "SocketCommands.h"
#include <iostream>
#include <string>
#include <fstream>
#include "Package.h"
#include "CommandParser.h"


namespace Udp
{
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

	int GetReadPackageFromBufferSize(const Session session)
	{
		if (session.LastPosition < session.BufferStartPosition)
			return 0;
		int bufferPosition = session.LastPosition - session.BufferStartPosition;
		return bufferPosition < MAX_DATA_LENGTH
			? bufferPosition
			: MAX_DATA_LENGTH;
	}

	Package ReadPackageFromBuffer(Session &session)
	{
		Package package;
		package.Number = session.LastPosition / MAX_DATA_LENGTH + 1;
		int bufferPosition = session.LastPosition - session.BufferStartPosition;
		package.Length = session.ReadBufferLength - bufferPosition < MAX_DATA_LENGTH
			? session.ReadBufferLength - bufferPosition
			: MAX_DATA_LENGTH;
		memcpy(package.Data, session.ReadBuffer + bufferPosition, package.Length);
		session.LastPosition += package.Length;
		return package;
	}

	int SendPackage(SOCKET socket, sockaddr_in toAddr, Package package)
	{
		return sendto(socket, reinterpret_cast<char*>(&package), PACKAGE_LENGTH, 0,
			reinterpret_cast<sockaddr*>(&toAddr), sizeof(toAddr));
	}

	int SendSocketPackege(Session &session)
	{
		if (session.LastPosition < session.FileSize)
		{
			int sendSize = session.FileSize - session.LastPosition < MAX_DATA_LENGTH
				? session.FileSize - session.LastPosition
				: MAX_DATA_LENGTH;
			if (sendSize > session.ReadBufferLength - (session.LastPosition - session.BufferStartPosition)
				|| session.LastPosition < session.BufferStartPosition)
			{
				WriteFileToBuffer(session);
			}
			Package package = ReadPackageFromBuffer(session); 
			int ret = SendPackage(session.ClientSocket, session.Sin, package);
			cout << "send: " << package.Number << endl;
			if (ret == SOCKET_ERROR)
			{
				throw ret;
			}
		}
		if (session.LastPosition == session.FileSize)
		{
			Package package;
			package.Number = session.LastPosition / MAX_DATA_LENGTH + 2;
			SendPackage(session.ClientSocket, session.Sin, package);
		}
		return 1;
	}

	int SendFile(Session &session)
	{
		timeval tv;
		fd_set set;

		while (true)
		{
			FD_ZERO(&set);
			FD_SET(session.ClientSocket, &set);
			int activity = select(session.ClientSocket + 1, &set, NULL, NULL, &tv);
			if ((activity < 0) && (errno != EINTR))
			{
				ShowMessage("select error");
			}
			if (activity > 0)
			{
				CommandParser parser;
				string message = ReadSocketMessage(session.ClientSocket, session.Sin);
				parser.setMessage(message);
				if (parser.getCommand() == "lastPosition")
				{
					cout << "LastPosition: " << parser.getParam(1) << " | "
						<< (float)(atoi(parser.getParam(1).c_str()) / MAX_DATA_LENGTH) << endl;
					session.LastPosition = atoi(parser.getParam(1).c_str());
				}
				else if (parser.getCommand() == "success")
				{
					session.clearSessionData();
					ShowMessage("Success");
					SendSocketMessage(session.ClientSocket, session.Sin, "connect");
					break;
				}
			}
			Udp::SendSocketPackege(session);
		}
		return 1;
	}

	//read
	void WriteBufferToFile(Session &session)
	{
		fstream file;
		file.open(session.FilePath, ios::out | ios::binary | ios::ate);
		if (!file.is_open()) return;
		file.seekg(session.LastPosition - session.ReadBufferLength, ios_base::end);
		file.write(session.ReadBuffer, session.ReadBufferLength);
		session.ReadBufferLength = 0;
		file.close();
	}

	void WritePackageToBuffer(Session &session, Package package)
	{
		memcpy(session.ReadBuffer + session.ReadBufferLength, package.Data, package.Length);
		session.ReadBufferLength += package.Length;
		session.LastPosition += package.Length;
	}

	int ReadPackege(SOCKET socket, sockaddr_in fromAddr, Package &package)
	{
		int fromLen = sizeof(fromAddr);
		return recvfrom(socket, reinterpret_cast<char*>(&package), PACKAGE_LENGTH, 0,
			reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
	}

	int ReadSocketPackege(Session &session)
	{
		if (session.LastPosition < session.FileSize)
		{
			Package package;
			int ret = ReadPackege(session.ClientSocket, session.Sin, package);
			if (ret == SOCKET_ERROR)
			{
				throw ret;
			}
			if ((int)(session.LastPosition / MAX_DATA_LENGTH) + 1 != package.Number)
			{
				if ((int)(session.LastPosition / MAX_DATA_LENGTH) + 1 > package.Number)
				{
					return 0;
				}
				if (!session.isAwaitPackage)
				{
					session.isAwaitPackage = true;
					SendSocketMessage(session.ClientSocket, session.Sin, string("lastPosition ") += to_string(session.LastPosition));
				}
				return 0;
			}
			session.isAwaitPackage = false;
			if (session.ReadBufferLength < session.MaxReadBufferLength - MAX_DATA_LENGTH)
			{
				WritePackageToBuffer(session, package);
			}
			if (session.ReadBufferLength >= session.MaxReadBufferLength - MAX_DATA_LENGTH)
			{
				WriteBufferToFile(session);
				WritePackageToBuffer(session, package);
			}
		}
		if (session.LastPosition == session.FileSize)
		{
			WriteBufferToFile(session);
			session.clearSessionData();
			SendSocketMessage(session.ClientSocket, session.Sin, "success");
			return 1;
		}
		return 0;
	}

	void CleanDebris(const Session session)
	{
		char buffer[1024];
		sockaddr_in sin = session.Sin;
		int size = sizeof(sin);
		while (true)
		{
			int result = recvfrom(session.ClientSocket, buffer, 1024, MSG_PEEK,
				reinterpret_cast<sockaddr*>(&sin), &size);
			if (result == SOCKET_ERROR)
			{
				recvfrom(session.ClientSocket, buffer, 1024, 0,
					reinterpret_cast<sockaddr*>(&sin), &size);
			}
			else
				return;
		}
	}

	int ReadFile(Session &session)
	{
		timeval tv;
		fd_set set;
		while (true)
		{
			FD_ZERO(&set);
			FD_SET(session.ClientSocket, &set);
			int activity = select(session.ClientSocket + 1, &set, NULL, NULL, &tv);
			if ((activity < 0) && (errno != EINTR))
			{
				ShowMessage("select error");
			}
			if (activity > 0)
			{
				if (Udp::ReadSocketPackege(session) == 1)
				{
					session.clearSessionData();
					break;
				}
			}
		}
		CleanDebris(session);
		ShowMessage("Success");
		return 1;
	}
}
