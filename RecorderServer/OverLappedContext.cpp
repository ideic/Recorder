#include "stdafx.h"
#include "OverLappedContext.h"

#define BUF_SIZE 2*1024

OverLappedContext::OverLappedContext():OverLappedContext(NULL) {

}
OverLappedContext::OverLappedContext(SOCKET socket):_socket(socket), _receivedBytes(0), _flags(0), _fromLength(sizeof(sockaddr_in))
{
	hEvent = 0;
	_buffer.len = BUF_SIZE;
	_buffer.buf = new char[BUF_SIZE];

	_backBuffer.len = BUF_SIZE;
	_backBuffer.buf = new char[BUF_SIZE];

	memset(&_from, 0, sizeof(sockaddr_in));
}

OverLappedContext::~OverLappedContext()
{
	delete[] _buffer.buf;
	delete[] _backBuffer.buf;

}
