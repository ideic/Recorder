#pragma once

#include <WinSock2.h>
#include <windows.h>
class OverLappedContext : public WSAOVERLAPPED
{
private:
	WSABUF _buffer, _backBuffer;
	SOCKET _socket;
	DWORD _receivedBytes;
	DWORD _flags;
	sockaddr_in  _from;
	int _fromLength;
	HANDLE _eventHandle;

public:
	OverLappedContext();
	OverLappedContext(SOCKET socket);
	//OverLappedContext(const OverLappedContext& ctx) = default;
	//OverLappedContext & operator=(const OverLappedContext& ctx) = default;

	~OverLappedContext();

	SOCKET& Socket() { return _socket; }
	WSABUF& Buffer() { return _buffer; }
	WSABUF& BackBuffer() { return _backBuffer; }
	DWORD& ReceivedBytes() { return _receivedBytes; }
	DWORD& Flags() { return _flags; }
	sockaddr_in & From() { return _from; }
	int& FromLength() { return _fromLength; }
};

