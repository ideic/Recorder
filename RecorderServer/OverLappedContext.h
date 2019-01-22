#pragma once

#include <WinSock2.h>
#include <windows.h>
class OverLappedContext : public OVERLAPPED
{
private:


public:
	OverLappedContext();
	OverLappedContext(SOCKET socket);
	//OverLappedContext(const OverLappedContext& ctx) = default;
	//OverLappedContext & operator=(const OverLappedContext& ctx) = default;

	~OverLappedContext();

	void ResetBuffer();

	WSABUF Buffer;
	SOCKET Socket;
	DWORD ReceivedBytes;
	DWORD Flags;
	sockaddr_in  From;
	int FromLength;
};

