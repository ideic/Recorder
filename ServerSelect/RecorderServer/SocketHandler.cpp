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

void SocketHandler::CreateSocket(const std::string &host, int16_t portNumber)
{
	struct addrinfo hints, *addrInfoInit;
	ZeroMemory(&hints, sizeof(hints));

	//hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int iResult = getaddrinfo(host.c_str(), std::to_string(portNumber).c_str(), &hints, &addrInfoInit);
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

	u_long NonBlock = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();
		throw std::runtime_error("Cannot set socket ioctlsocket async io. ErrorCode:" + std::to_string(iResult));

	}



	if (bind(listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR) {
		iResult = WSAGetLastError();
		throw std::runtime_error("Cannot bind socket to local address. Error:" + iResult);
	}



	Ctx = std::make_shared<SocketOverLappedContext>();
	Ctx->Socket = listenSocket;
	Ctx->DstPort = portNumber;

	/*wchar_t ip[INET_ADDRSTRLEN];
	InetNtop(addrInfo->ai_family, &addrInfo->ai_addr, ip, INET_ADDRSTRLEN);
	*/
	Ctx->DstIp = inet_ntoa(((sockaddr_in*)addrInfo->ai_addr)->sin_addr);


}
