#include "stdafx.h"
#include "RecorderServer.h"

#include <stdexcept>
#include <algorithm>
#include "ws2tcpip.h"
#include <string>
#include <functional>


RecorderServer::RecorderServer()
{
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw std::runtime_error("WSAStartup failed with error: " + iResult);
	}
}

RecorderServer::~RecorderServer()
{
	WSACleanup();
}

void RecorderServer::StartServer(const std::vector<std::string>& endpoints)
{
	_endpoints = endpoints;

	_completionPort.reset(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0));

	if (_completionPort.get() == NULL) {
		throw std::runtime_error("IO Completion port create failed with error: " + GetLastError());
	}

	StartWorkers(1);

	std::for_each(endpoints.begin(), endpoints.end(), [this](const std::string &port) {
		CreatePort(port);
	});
}

void RecorderServer::StartWorkers(uint8_t numberOfThreads)
{
	for (int i = 0; i < numberOfThreads; ++i) {
		_workers.emplace_back([&]() {Worker(); });
	}
}

void RecorderServer::Worker() {
	while (1) {

		DWORD numberOfBytes= 0;
		unsigned long long completionKey = 0;
		LPOVERLAPPED ctx = 0;

		auto c = _completionPort.get();

		auto ioSucceeds = GetQueuedCompletionStatus(
			_completionPort.get(),
			&numberOfBytes,
			&completionKey,
			&ctx,
			10000 //  dwMilliseconds
		);

		if (ioSucceeds) {
			if (&numberOfBytes > 0) {

				auto overlappedContext = (OverLappedContext*)ctx;
		
				overlappedContext->ResetBuffer();

				int iresult = WSARecvFrom(overlappedContext->Socket(), &overlappedContext->Buffer(), 1, &overlappedContext->ReceivedBytes(), &overlappedContext->Flags(), (sockaddr*)& overlappedContext->From(), &overlappedContext->FromLength(), overlappedContext, NULL);
				
				if (iresult != 0) {
					iresult = WSAGetLastError();
					if (iresult != WSA_IO_PENDING) {
						//socket closed;
						iresult = 0;
					}
				}
			}
		}
		else {
			auto iResult = WSAGetLastError();

			if (WSAGetLastError() != WAIT_TIMEOUT)
			{
				// Init Receive ?
			}
		}

	}
}

void RecorderServer::CreatePort(std::string port) {

	int iport = std::stoi(port);

	struct addrinfo hints, *addrInfoInit;
	ZeroMemory(&hints, sizeof(hints));

	//hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int iResult = getaddrinfo(NULL, port.c_str(), &hints, &addrInfoInit);
	if (iResult != 0) {
		throw std::runtime_error("getaddrinfo failed with error:" + iResult);
	}

	std::shared_ptr<addrinfo> addrInfo(addrInfoInit, freeaddrinfo);

	auto listenSocket = WSASocket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		throw std::runtime_error("socket failed with error: " + WSAGetLastError());
	}

	char opt = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &opt, sizeof(opt)))
	{
		iResult = WSAGetLastError();
		throw std::runtime_error("Cannot set socket to use exclusive address. ErrorCode:" + iResult);
	}

	opt = 0;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		iResult = WSAGetLastError();
		throw std::runtime_error("Cannot set socket to don't reuse address. ErrorCode:" + iResult);
	}

	if (bind(listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR) {
		iResult = WSAGetLastError();
		throw std::runtime_error("Cannot bind socket to local address. Error:" + iResult);
	}


	auto listenPort = CreateIoCompletionPort((HANDLE)listenSocket, _completionPort.get(), iport, 0);

	if (listenPort == 0) {
		iResult = WSAGetLastError();
		closesocket(listenSocket);
		throw std::runtime_error("Cannot set socket to use exclusive address. ErrorCode:" + iResult);
	}

	_openPorts.emplace_back(std::make_shared<OverLappedContext>(listenSocket, listenPort));

	auto & ctx = _openPorts[0];
	ctx->ResetBuffer();

	int iresult = WSARecvFrom(_openPorts[0]->Socket(), &ctx->Buffer(), 1, &ctx->ReceivedBytes(), &ctx->Flags(), (sockaddr*)& ctx->From(), &ctx->FromLength(), ctx.get(), NULL);
	iresult = WSAGetLastError();

}

void RecorderServer::StopServer()
{
	_completionPort.release();

	std::for_each(_openPorts.begin(), _openPorts.end(), [](std::shared_ptr<OverLappedContext> &port) {
		closesocket(port->Socket());
	});

	_openPorts.clear();
	_endpoints.clear();
}
