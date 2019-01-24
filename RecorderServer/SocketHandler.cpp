#include "stdafx.h"
#include "SocketHandler.h"
#include <string>
#include "ws2tcpip.h"
#include <memory>

SocketHandler::SocketHandler()
{
}


SocketHandler::~SocketHandler()
{
	if (Ctx != nullptr) {
		closesocket(Ctx->Socket);
	}
}

void SocketHandler::CreateSocket(int16_t portNumber, HANDLE completionPort)
{
	struct addrinfo hints, *addrInfoInit;
	ZeroMemory(&hints, sizeof(hints));

	//hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int iResult = getaddrinfo("172.16.81.12", std::to_string(portNumber).c_str(), &hints, &addrInfoInit);
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


	auto listenPort = CreateIoCompletionPort((HANDLE)listenSocket, completionPort, portNumber, 0);

	if (listenPort == 0) {
		iResult = WSAGetLastError();
		closesocket(listenSocket);
		throw std::runtime_error("Cannot set socket to use exclusive address. ErrorCode:" + iResult);
	}

	Ctx = std::make_shared<SocketOverLappedContext>();
	Ctx->Socket = listenSocket;
	Ctx->IOPort = listenPort;
	Ctx->DstPort = portNumber;

	/*wchar_t ip[INET_ADDRSTRLEN];
	InetNtop(addrInfo->ai_family, &addrInfo->ai_addr, ip, INET_ADDRSTRLEN);
	*/
	Ctx->DstIp = inet_ntoa(((sockaddr_in*)addrInfo->ai_addr)->sin_addr);
}
