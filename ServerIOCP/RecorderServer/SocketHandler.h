#pragma once
#include <WinSock2.h>
#include <windows.h>
#include "SocketOverLappedContext.h"
#include <memory>
#include <string>

class SocketHandler
{
public:
	SocketHandler();
	~SocketHandler();

	void CreateSocket(const std::string &host,int16_t portNumber, HANDLE completionPort);

	std::shared_ptr<SocketOverLappedContext> Ctx;
};

