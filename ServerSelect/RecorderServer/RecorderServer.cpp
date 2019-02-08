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
	//CloseHandle(_completionPort);
	WSACleanup();
}

void RecorderServer::StartServer(const std::string &host, const std::vector<int>& ports, uint8_t numberOfThreads, std::wstring workDir)
{

	_fileServer = std::make_unique<FileServer>(workDir);
	_terminate = false;

	LoggerFactory::Logger()->LogInfo("Start RecordingServer Worker threads:" + std::to_string(numberOfThreads));

	

	LoggerFactory::Logger()->LogInfo("Start FileServer Worker. Threads:" + std::to_string(numberOfThreads));

	_fileServer->StartServer(numberOfThreads);

	LoggerFactory::Logger()->LogInfo("Start Listening");

	std::for_each(ports.begin(), ports.end(), [this, &host](const int port) {
		CreatePort(host, port);
	});

	LoggerFactory::Logger()->LogInfo("RecordingServer is running");

	StartWorkers(numberOfThreads);
}

void RecorderServer::StartWorkers(uint8_t numberOfThreads)
{
	//Worker();
	numberOfThreads = 1;
	for (int i = 0; i < numberOfThreads; ++i) {
		_workers.emplace_back([&]() {Worker(); });
	}
}


void RecorderServer::Worker() {
	MSG msg;

	FD_SET ReadSet;
	FD_ZERO(&ReadSet);

	for (auto & openPort: _openPorts){
		FD_SET(openPort.second->Ctx->Socket, &ReadSet);
	}

	TIMEVAL tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	while (!_terminate) {
		int total = 0;

		if ((total = select(0, &ReadSet, NULL, NULL, &tv)) == SOCKET_ERROR) {
			LoggerFactory::Logger()->LogWarning("Socket error at select: " + std::to_string(WSAGetLastError()));
			continue;
		}

		if (total == 0) { //Timeout
			FD_ZERO(&ReadSet);

			for (auto &openPort : _openPorts) {
				FD_SET(openPort.second->Ctx->Socket, &ReadSet);
			}
		}

		for (int i = 0; i < total; i++) {
			auto found = _openPorts.find((SOCKET)ReadSet.fd_array[i]);

			if (found == std::end(_openPorts)) {
				LoggerFactory::Logger()->LogWarning("Socket not found ");
				continue;
			}

			auto& overlappedContext = found->second->Ctx;

			overlappedContext->ResetBuffer();

			int iresult = WSARecvFrom(overlappedContext->Socket, &overlappedContext->Buffer, 1, &overlappedContext->ReceivedBytes, &overlappedContext->Flags, (sockaddr*)& overlappedContext->From, &overlappedContext->FromLength, NULL, NULL);

			if (iresult != 0) {
				iresult = WSAGetLastError();
				if (iresult != WSAEWOULDBLOCK) {
					LoggerFactory::Logger()->LogWarning("RecordingServer WindowHandle 1st WSARecvFrom init failed with Code:" + iresult);
					//socket closed;
					continue;
				}
			}

			_fileServer->SaveData(overlappedContext->Buffer, overlappedContext->ReceivedBytes, overlappedContext->From, overlappedContext->DstIp, overlappedContext->DstPort);

		}
	}
}

void RecorderServer::CreatePort(const std::string &host, int port) {

	std::shared_ptr<SocketHandler>  socket = std::make_shared<SocketHandler>();

	try
	{

		socket->CreateSocket(host, port);

		_openPorts.emplace(socket->Ctx->Socket,socket);
	}
	catch (const std::exception& e)
	{
		LoggerFactory::Logger()->LogError(e, "Create socker error on host:" + host + "port:" + std::to_string(port));
		return;
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
