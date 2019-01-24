#include "asyncudpsocket.h"
#include <iostream>
#include <string>
#include <WinSock2.h>
#include "completionporthandler.h"


using namespace std;


AsyncUdpSocket::AsyncUdpSocket(CompletionPortHandler& completionPortHandler, const sockaddr* remoteAddress) :
	remoteAddress(remoteAddress)
{
	socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == socket ) {
		throw runtime_error("socket() failed " + to_string(WSAGetLastError()));
	}

	completionPortHandler.attachHandler(*this);
}

AsyncUdpSocket::~AsyncUdpSocket() {
	shutdown(socket, SD_BOTH);
	closesocket(socket);

	if (!perIoDatas.empty()) {
		cerr << "AsyncUdpSocket::~AsyncUdpSocket()  !perIoDatas.empty()" << endl;
	}
}

HANDLE AsyncUdpSocket::getHandle() const {
	return reinterpret_cast<HANDLE>(socket);
}

void AsyncUdpSocket::sendUdpPacket(const UdpPacketDataPtr& udpPacketData) {
	shared_ptr<PerIoData> perIoData(new PerIoData());

	{
		lock_guard<mutex> lock(mtx);
		perIoDatas.insert(make_pair(perIoData.get(), perIoData));
	}
	
	perIoData->wsaBuffer.len = (ULONG)udpPacketData->length;
	perIoData->wsaBuffer.buf = (CHAR*)udpPacketData->buffer.get();

	send(perIoData.get());
}

void AsyncUdpSocket::send(PerIoData* perIoData) {
	static const DWORD flags = 0;
	bool sendSuccess = false;

	memset(&perIoData->overlapped, 0, sizeof(perIoData->overlapped));

	while (!sendSuccess) {
		if (SOCKET_ERROR == WSASendTo(socket, &perIoData->wsaBuffer, 1, NULL, flags, remoteAddress, sizeof(sockaddr), (OVERLAPPED*)perIoData, NULL)) {
			int lastError = WSAGetLastError();
			if (lastError != ERROR_IO_PENDING) {
				throw runtime_error("sendto() failed " + to_string(lastError));
			}

			this_thread::yield();
		}
		else {
			sendSuccess = true;
		}
	}
}

void AsyncUdpSocket::onCompletion(unsigned long transferred, PerIoData* perIoData) {
	//cout << "AsyncUdpSocket::onCompletion(" << transferred << ")" << endl;


	if (transferred > perIoData->wsaBuffer.len) {
		throw logic_error("AsyncUdpSocket::onCompletion()  transferred > perIoData->wsaBuffer.len");
	}

	if (transferred == perIoData->wsaBuffer.len) {
		lock_guard<mutex> lock(mtx);
		auto it = perIoDatas.find(perIoData);
		if (perIoDatas.end() == it) {
			throw logic_error("AsyncUdpSocket::onCompletion()  perIoDatas.end() == it");
		}
		perIoDatas.erase(it);
	}
	else {
		perIoData->wsaBuffer.len -= transferred;
		perIoData->wsaBuffer.buf += transferred;
		send(perIoData);
	}
}
