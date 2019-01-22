#include "stdafx.h"
#include "OverLappedContext.h"

#define BUF_SIZE 2*1024

OverLappedContext::OverLappedContext():OverLappedContext(NULL) {

}
OverLappedContext::OverLappedContext(SOCKET socket):Socket(socket), ReceivedBytes(0), Flags(0), FromLength(sizeof(sockaddr_in))
{
	hEvent = 0;
	Buffer.buf = nullptr;
}

void OverLappedContext::ResetBuffer() {
	if (Buffer.buf != nullptr) {
		delete[] Buffer.buf;
		Buffer.len = 0;
	}

	Buffer.len = BUF_SIZE;
	Buffer.buf = new char[BUF_SIZE];


	memset(&From, 0, sizeof(sockaddr_in));
}

OverLappedContext::~OverLappedContext()
{
	delete[] Buffer.buf;
	Buffer.buf = nullptr;
}
