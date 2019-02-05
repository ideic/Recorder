#include "asyncudpsocket.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
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
		cerr << "AsyncUdpSocket::~AsyncUdpSocket()  !contexts.empty()" << endl;
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
	bool sendSuccess = false;
	int tryIdx = 0;

	memset(context, 0, sizeof(OVERLAPPED));

	while (!sendSuccess) {
		if (SOCKET_ERROR == WSASendTo(socket, &context->wsaBuffer, 1, NULL, flags, &remoteAddress, sizeof(sockaddr), context, NULL)) {
			int lastError = WSAGetLastError();
			if (lastError != ERROR_IO_PENDING) {
				tryIdx++;
				if (tryIdx > 100) {
					throw runtime_error("sendto() failed " + to_string(lastError));
				}
			}

			this_thread::yield();
		}
		else {
			tryIdx = 0;
			sendSuccess = true;
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
	struct hostent *hp = gethostbyname(remoteHost.c_str());
	if (NULL == hp) {
		throw runtime_error("Can not resolve address: " + remoteHost);
	}

	struct sockaddr_in remoteAddress;
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(port + portIncrement);
	memset(&remoteAddress.sin_addr, 0, sizeof(remoteAddress.sin_addr));
	memcpy_s(&remoteAddress.sin_addr, sizeof(remoteAddress.sin_addr), hp->h_addr, hp->h_length);

	portIncrement += 2;

	return shared_ptr<AsyncUdpSocket>(new AsyncUdpSocket(completionPortHandler, reinterpret_cast<const sockaddr*>(&remoteAddress)));
}
