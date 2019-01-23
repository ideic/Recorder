#pragma once

#include <WinSock2.h>
#include <windows.h>
class SocketOverLappedContext : public OVERLAPPED
{
private:


public:
	SocketOverLappedContext();
	SocketOverLappedContext(SOCKET socket);
	//OverLappedContext(const OverLappedContext& ctx) = default;
	//OverLappedContext & operator=(const OverLappedContext& ctx) = default;

	~SocketOverLappedContext();

	void ResetBuffer();

	WSABUF Buffer;
	SOCKET Socket;
	DWORD ReceivedBytes;
	DWORD Flags;
	sockaddr_in  From;
	int FromLength;
	HANDLE IOPort;
};

