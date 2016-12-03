#pragma once
#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <WinSock2.h>
#include <WinSock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#elif defined(__linux__) || defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define nullptr NULL
#define to_string atoi
#else
#error "Uknown platform"
#endif
