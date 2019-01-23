#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <thread>

#include "SocketHandler.h"
#include "FileServer.h"

#include <WinSock2.h>
#include <windows.h>

class RecorderServer {
private:
	bool _terminate;
	std::vector<std::string> _endpoints;
	std::vector<std::shared_ptr<SocketHandler>> _openPorts;
	std::shared_ptr < void> _completionPort{NULL, CloseHandle};
	std::vector<std::thread> _workers;
	std::unique_ptr<FileServer> _fileServer;

	void CreatePort(std::string port);
	void Worker();
	void StartWorkers(uint8_t numberOfThreads);
	

public:
	RecorderServer();
	~RecorderServer();

	void StartServer(const std::vector<std::string> &ports, uint8_t numberOfThreads, std::wstring workDir);
	

	void StopServer();
};