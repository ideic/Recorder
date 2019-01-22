#include "stdafx.h"
#include "OverLappedContext.h"

#define BUF_SIZE 2*1024

OverLappedContext::OverLappedContext():OverLappedContext(NULL, nullptr) {

}
OverLappedContext::OverLappedContext(SOCKET socket, HANDLE listenIoPort):_socket(socket), _receivedBytes(0), _flags(0), _fromLength(sizeof(sockaddr_in)), _listenIoPort(listenIoPort)
{
	hEvent = 0;
	_buffer.buf = nullptr;
}

void OverLappedContext::ResetBuffer() {
	if (_buffer.buf != nullptr) {
		delete[] _buffer.buf;
		_buffer.len = 0;
	}

	_buffer.len = BUF_SIZE;
	_buffer.buf = new char[BUF_SIZE];


	memset(&_from, 0, sizeof(sockaddr_in));
}

OverLappedContext::~OverLappedContext()
{
	
	//if (_listenIoPort != nullptr) {
	//	CloseHandle(_listenIoPort);
	//	_listenIoPort = nullptr;
	//}

	//closesocket(_socket);
}
