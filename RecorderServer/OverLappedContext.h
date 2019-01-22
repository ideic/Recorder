#pragma once

#include <WinSock2.h>
#include <windows.h>
class OverLappedContext : public OVERLAPPED
{
private:
	WSABUF _buffer;
	SOCKET _socket;
	DWORD _receivedBytes;
	DWORD _flags;
	sockaddr_in  _from;
	int _fromLength;
	HANDLE _listenIoPort;

public:
	OverLappedContext();
	OverLappedContext(SOCKET socket, HANDLE listenIoPort);
	//OverLappedContext(const OverLappedContext& ctx) = default;
	//OverLappedContext & operator=(const OverLappedContext& ctx) = default;

	~OverLappedContext();

	void ResetBuffer();

	SOCKET& Socket() { return _socket; }
	WSABUF& Buffer() { return _buffer; }
	DWORD& ReceivedBytes() { return _receivedBytes; }
	DWORD& Flags() { return _flags; }
	sockaddr_in & From() { return _from; }
	int& FromLength() { return _fromLength; }
};

