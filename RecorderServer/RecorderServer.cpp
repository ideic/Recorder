#include "stdafx.h"
#include "RecorderServer.h"

#include <stdexcept>
#include <algorithm>
#include "ws2tcpip.h"
#include <string>
#include <functional>


RecorderServer::RecorderServer():_terminate(false)
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

void RecorderServer::StartServer(const std::vector<std::string>& endpoints, uint8_t numberOfThreads, std::wstring workDir)
{
	_endpoints = endpoints;
	_fileServer = std::make_unique<FileServer>(workDir);
	_terminate = false;

	_completionPort.reset(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0));

	if (_completionPort.get() == NULL) {
		throw std::runtime_error("IO Completion port create failed with error: " + GetLastError());
	}

	StartWorkers(numberOfThreads);
	_fileServer->StartServer(numberOfThreads);

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
	while (!_terminate) {

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

				auto overlappedContext = (SocketOverLappedContext*)ctx;
		
				_fileServer->SaveData(overlappedContext->Buffer, numberOfBytes, overlappedContext->From);

				overlappedContext->ResetBuffer();

				int iresult = WSARecvFrom(overlappedContext->Socket, &overlappedContext->Buffer, 1, &overlappedContext->ReceivedBytes, &overlappedContext->Flags, (sockaddr*)& overlappedContext->From, &overlappedContext->FromLength, overlappedContext, NULL);
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

	std::shared_ptr<SocketHandler>  socket = std::make_shared<SocketHandler>();

	socket->CreateSocket(std::stoi(port), _completionPort);

	_openPorts.push_back(socket);

	auto & ctx = socket->Ctx;
	ctx->ResetBuffer();

	int iresult = WSARecvFrom(ctx->Socket, &ctx->Buffer, 1, &ctx->ReceivedBytes, &ctx->Flags, (sockaddr*)& ctx->From, &ctx->FromLength, ctx.get(), NULL);
	iresult = WSAGetLastError();

}

void RecorderServer::StopServer()
{
	_terminate = true;
	for (auto& worker : _workers) {
		worker.join();
	};

	_fileServer->StopServer();
	_completionPort.reset();

	_openPorts.clear();
	_endpoints.clear();
	_workers.clear();
}
