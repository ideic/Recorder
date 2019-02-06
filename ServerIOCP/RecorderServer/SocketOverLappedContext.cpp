#include "stdafx.h"
#include "SocketOverLappedContext.h"

#define BUF_SIZE 2*1024

SocketOverLappedContext::SocketOverLappedContext():SocketOverLappedContext(NULL) {

}
SocketOverLappedContext::SocketOverLappedContext(SOCKET socket):Socket(socket), ReceivedBytes(0), Flags(0), FromLength(sizeof(sockaddr_in))
{
	hEvent = 0;
	Buffer.buf = nullptr;
}

void SocketOverLappedContext::ResetBuffer() {
	if (Buffer.buf != nullptr) {
		delete[] Buffer.buf;
		Buffer.len = 0;
	}

	Buffer.len = BUF_SIZE;
	Buffer.buf = new char[BUF_SIZE];


	memset(&From, 0, sizeof(sockaddr_in));
}

SocketOverLappedContext::~SocketOverLappedContext()
{
	delete[] Buffer.buf;
	Buffer.buf = nullptr;
}
