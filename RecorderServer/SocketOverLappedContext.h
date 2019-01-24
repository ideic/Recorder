#pragma once

#include <WinSock2.h>
#include <windows.h>
#include <stdint.h>
#include <string>
class SocketOverLappedContext : public OVERLAPPED
{
private:


public:
	SocketOverLappedContext();
	SocketOverLappedContext(SOCKET socket);

	~SocketOverLappedContext();

	void ResetBuffer();

	WSABUF Buffer;
	SOCKET Socket;
	DWORD ReceivedBytes;
	DWORD Flags;
	sockaddr_in  From;
	int FromLength;
	HANDLE IOPort;

	int16_t DstPort;
	std::string DstIp;
};

