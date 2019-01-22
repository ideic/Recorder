#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <thread>

#include "OverLappedContext.h"

#include <WinSock2.h>
#include <windows.h>

class RecorderServer {
private:
	std::vector<std::string> _endpoints;
	std::vector<std::shared_ptr<OverLappedContext>> _openPorts;
	std::unique_ptr < void, std::function<void(HANDLE)>> _completionPort{NULL, CloseHandle};
	std::vector<std::thread> _workers;

	void CreatePort(std::string port);
	void Worker();
	void StartWorkers(uint8_t numberOfThreads);
	

public:
	RecorderServer();
	~RecorderServer();

	void StartServer(const std::vector<std::string> &ports);
	

	void StopServer();
};