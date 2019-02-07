#include "asyncudpsocket.h"
#include <iostream>
#include <ws2tcpip.h>
#include "completionporthandler.h"

using namespace std;


AsyncUdpSocket::AsyncUdpSocket(CompletionPortHandler& completionPortHandler, const sockaddr* remoteAddress) {
	socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == socket ) {
		throw runtime_error("socket() failed " + to_string(WSAGetLastError()));
	}

	memcpy_s(&this->remoteAddress, sizeof(this->remoteAddress), remoteAddress, sizeof(*remoteAddress));
	completionPortHandler.attachHandler(*this);
}

AsyncUdpSocket::~AsyncUdpSocket() {
	shutdown(socket, SD_BOTH);
	closesocket(socket);

	if (!contexts.empty()) {
		cerr << "AsyncUdpSocket::~AsyncUdpSocket()  !contexts.empty()";
	}
}

HANDLE AsyncUdpSocket::getHandle() const {
	return reinterpret_cast<HANDLE>(socket);
}

void AsyncUdpSocket::sendUdpPacket(const UdpPacketDataPtr& udpPacketData) {
	shared_ptr<struct Context> context(new Context());

	{
		lock_guard<mutex> lock(mtx);
		contexts.insert(make_pair(context.get(), make_pair(context, udpPacketData)));
	}
	
	context->wsaBuffer.len = (ULONG)udpPacketData->length;
	context->wsaBuffer.buf = (CHAR*)udpPacketData->buffer.get();

	send(context.get());
}

void AsyncUdpSocket::send(struct Context* context) {
	static const DWORD flags = 0;

	memset(context, 0, sizeof(OVERLAPPED));
	
	if (SOCKET_ERROR == WSASendTo(socket, &context->wsaBuffer, 1, NULL, flags, &remoteAddress, sizeof(sockaddr), context, NULL)) {
		const int lastError = WSAGetLastError();
		if (lastError != ERROR_IO_PENDING) {
			throw runtime_error("sendto() failed " + to_string(lastError));
		}
	}
}

void AsyncUdpSocket::onCompletion(unsigned long transferred, struct Context* context) {
	if (transferred > context->wsaBuffer.len) {
		throw logic_error("AsyncUdpSocket::onCompletion()  transferred > context->wsaBuffer.len");
	}

	if (transferred == context->wsaBuffer.len) {
		lock_guard<mutex> lock(mtx);
		auto it = contexts.find(context);
		if (contexts.end() == it) {
			throw logic_error("AsyncUdpSocket::onCompletion()  contexts.end() == it");
		}
		contexts.erase(it);
	}
	else {
		context->wsaBuffer.len -= transferred;
		context->wsaBuffer.buf += transferred;
		send(context);
	}
}

AsyncUdpSocketFactory::AsyncUdpSocketFactory(CompletionPortHandler& completionPortHandler, const string& remoteHost, unsigned short port) : 
	completionPortHandler(completionPortHandler),
	remoteHost(remoteHost),
	port(port),
	portIncrement(0)
{
}

shared_ptr<AsyncUdpSocket> AsyncUdpSocketFactory::create() {
	struct addrinfo hints, *addrInfoInit;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	if (0 != getaddrinfo(remoteHost.c_str(), std::to_string(port + portIncrement).c_str(), &hints, &addrInfoInit)) {
		throw runtime_error("getaddrinfo failed with error:" + to_string(WSAGetLastError()));
	}

	portIncrement += 2;

	std::shared_ptr<addrinfo> addrInfo(addrInfoInit, freeaddrinfo);
	return shared_ptr<AsyncUdpSocket>(new AsyncUdpSocket(completionPortHandler, addrInfo->ai_addr));
}
