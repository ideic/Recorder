#include "stdafx.h"
#include "RecorderServer.h"

#include <stdexcept>
#include <algorithm>
#include "ws2tcpip.h"
#include <string>


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


	std::for_each(endpoints.begin(), endpoints.end(), [this](const std::string &port) {
		CreatePort(port);
	});
}

void RecorderServer::ProcessRequests(uint8_t numberOfThreads)
{

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

	auto listenPort = CreateIoCompletionPort((HANDLE)listenSocket, _completionPort.get(), iport, 0);

	char opt = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &opt, sizeof(opt)))
	{
		iResult = WSAGetLastError();
		closesocket(listenSocket);
		throw std::runtime_error("Cannot set socket to use exclusive address. ErrorCode:" + iResult);
	}

	opt = 0;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		iResult = WSAGetLastError();
		closesocket(listenSocket);
		throw std::runtime_error("Cannot set socket to don't reuse address. ErrorCode:" + iResult);
	}

	if (bind(listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR) {
		iResult = WSAGetLastError();
		closesocket(listenSocket);
		throw std::runtime_error("Cannot bind socket to local address. Error:" + iResult);
	}

	_openPorts.emplace(iport, listenSocket);
	auto &ctx = _openPorts[iport];

	if (iResult = WSARecvFrom(ctx.Socket(), &ctx.BackBuffer(), 1, &ctx.ReceivedBytes(), &ctx.Flags(), (sockaddr*)&ctx.From(), &ctx.FromLength(), &ctx, NULL)) {
		iResult = WSAGetLastError();
		if (iResult != WSA_IO_PENDING) {
			closesocket(listenSocket);
			throw std::runtime_error("Received failed. Error:" + iResult);
		}
	}
}

void RecorderServer::StopServer()
{
	_completionPort.release();

	std::for_each(_openPorts.begin(), _openPorts.end(), [](std::pair<const int, OverLappedContext> &port) {
		closesocket(port.second.Socket());
	});

	_openPorts.clear();
	_endpoints.clear();
}
