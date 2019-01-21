#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "OverLappedContext.h"

#include <WinSock2.h>
#include <windows.h>

class RecorderServer {
private:
	std::vector<std::string> _endpoints;
	std::unordered_map<int, OverLappedContext> _openPorts;
	std::unique_ptr < void, std::function<void(HANDLE)>> _completionPort{NULL, CloseHandle};

	void CreatePort(std::string port);
public:
	RecorderServer();
	~RecorderServer();

	void StartServer(const std::vector<std::string> &ports);
	void ProcessRequests(uint8_t numberOfThreads);

	void StopServer();
};