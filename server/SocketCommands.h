#pragma once

#include <WinSock2.h>
#include <fstream>
#include "Session.h"

using namespace std;


void ShowMessage(string message);

int FileSize(string filePath);

string ReadSocketMessage(SOCKET socket);

int SendSocketMessage(SOCKET socket, string message);

int SendFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession);

int ReadFile(SOCKET socket, string filePath, int fileSize, int startPosition, Session* currentSession);