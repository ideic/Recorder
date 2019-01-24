#include "stdafx.h"
#include "RecorderServer.h"

#include <stdexcept>
#include <algorithm>
#include "ws2tcpip.h"
#include <string>
#include <functional>
#include "LoggerFactory.h"

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
	CloseHandle(_completionPort);
	WSACleanup();
}

void RecorderServer::StartServer(const std::string &host, const std::vector<int>& ports, uint8_t numberOfThreads, std::wstring workDir)
{

	_fileServer = std::make_unique<FileServer>(workDir);
	_terminate = false;

	_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	if (!_completionPort) {
		throw std::runtime_error("IO Completion port create failed with error: " + GetLastError());
	}

	LoggerFactory::Logger()->LogInfo("Start RecordingServer Worker threads:" + std::to_string(numberOfThreads));

	StartWorkers(numberOfThreads);

	LoggerFactory::Logger()->LogInfo("Start FileServer Worker. Threads:" + std::to_string(numberOfThreads));

	_fileServer->StartServer(numberOfThreads);

	LoggerFactory::Logger()->LogInfo("Start Listening");

	std::for_each(ports.begin(), ports.end(), [this, &host](const int port) {
		CreatePort(host, port);
	});

	LoggerFactory::Logger()->LogInfo("RecordingServer is running");
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

		auto ioSucceeds = GetQueuedCompletionStatus(
			_completionPort,
			&numberOfBytes,
			&completionKey,
			&ctx,
			1000 //  dwMilliseconds
		);

		if (ioSucceeds) {
			if (&numberOfBytes > 0) {

				auto overlappedContext = (SocketOverLappedContext*)ctx;
		
				_fileServer->SaveData(overlappedContext->Buffer, numberOfBytes, overlappedContext->From, overlappedContext->DstIp, overlappedContext->DstPort);

				overlappedContext->ResetBuffer();

				int iresult = WSARecvFrom(overlappedContext->Socket, &overlappedContext->Buffer, 1, &overlappedContext->ReceivedBytes, &overlappedContext->Flags, (sockaddr*)& overlappedContext->From, &overlappedContext->FromLength, overlappedContext, NULL);
				if (iresult != 0) {
					iresult = WSAGetLastError();
					if (iresult != WSA_IO_PENDING) {
						LoggerFactory::Logger()->LogWarning("RecordingServer IOCP 2nd WSARecvFrom init failed with Code:" + iresult);
						//socket closed;
						iresult = 0;
					}
				}
			}
		}
		else {
			auto iResult = WSAGetLastError();

			if (iResult != WAIT_TIMEOUT)
			{
				LoggerFactory::Logger()->LogWarning("RecordingServer IOCP GetQueuedCompletionStatus failed with Code:" + iResult);

				// Init Receive ?
			}
		}

	}
}

void RecorderServer::CreatePort(const std::string &host, int port) {

	std::shared_ptr<SocketHandler>  socket = std::make_shared<SocketHandler>();

	socket->CreateSocket(host, port, _completionPort);

	_openPorts.push_back(socket);

	auto & ctx = socket->Ctx;
	ctx->ResetBuffer();

	int iresult = WSARecvFrom(ctx->Socket, &ctx->Buffer, 1, &ctx->ReceivedBytes, &ctx->Flags, (sockaddr*)& ctx->From, &ctx->FromLength, ctx.get(), NULL);

	if (iresult != 0) {
		iresult = WSAGetLastError();
		if (iresult != WSA_IO_PENDING) {
			LoggerFactory::Logger()->LogWarning("RecordingServer IOCP 1st WSARecvFrom init failed with Code:" + iresult);
			//socket closed;
			iresult = 0;
		}
	}


}

void RecorderServer::StopServer()
{
	_terminate = true;
	LoggerFactory::Logger()->LogInfo("RecordingServer Stop Workers");

	for (auto& worker : _workers) {
		worker.join();
	};
	LoggerFactory::Logger()->LogInfo("RecordingServer Stop File Server");
	_fileServer->StopServer();

	LoggerFactory::Logger()->LogInfo("RecordingServer Clear OpenPorts");
	_openPorts.clear();

	LoggerFactory::Logger()->LogInfo("RecordingServer Clear Workers");
	_workers.clear();

}
