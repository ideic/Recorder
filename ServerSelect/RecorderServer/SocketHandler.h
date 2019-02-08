#pragma once
#include <WinSock2.h>
#include <windows.h>
#include "SocketOverLappedContext.h"
#include <memory>
#include <string>
#define WM_SOCKET (WM_USER + 1)
class SocketHandler
{
public:
	SocketHandler();
	~SocketHandler();

	void CreateSocket(const std::string &host,int16_t portNumber);

	std::shared_ptr<SocketOverLappedContext> Ctx;
};

