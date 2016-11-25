#pragma once

#include <WinSock2.h>
#include "Session.h"

using namespace std;


void ShowMessage(string message);

int FileSize(string filePath);

string ReadSocketMessage(SOCKET socket, sockaddr_in fromAddr);

int SendSocketPackege(Session &session);

int ReadSocketPackege(Session &session);

int SendSocketMessage(SOCKET socket, struct sockaddr_in toAddr, string message);

int SendFile(SOCKET socket, sockaddr_in toAddr, string filePath, int fileSize, int startPosition);

int ReadFile(SOCKET socket, sockaddr_in fromAddr, string filePath, int fileSize, int startPosition);