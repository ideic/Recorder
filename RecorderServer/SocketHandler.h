#pragma once
#include <WinSock2.h>
#include <windows.h>
#include "SocketOverLappedContext.h"
#include <memory>

class SocketHandler
{
public:
	SocketHandler();
	~SocketHandler();

	void CreateSocket(int16_t portNumber, std::shared_ptr<void> completionPort);

	std::shared_ptr<SocketOverLappedContext> Ctx;
};

