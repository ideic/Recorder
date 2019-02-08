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
#include <map>

class RecorderServer {
private:
	bool _terminate{false};
	std::unordered_map<SOCKET, std::shared_ptr<SocketHandler>> _openPorts;
	//HANDLE _completionPort{NULL};
	std::vector<std::thread> _workers;
	std::unique_ptr<FileServer> _fileServer;

	void CreatePort(const std::string &host, int port);
	void Worker();
	void StartWorkers(uint8_t numberOfThreads);

	FD_SET _readSet;
public:
	RecorderServer();
	~RecorderServer();

	void StartServer(const std::string &host,  const std::vector<int> &ports, uint8_t numberOfThreads, std::wstring workDir);
	

	void StopServer();
};