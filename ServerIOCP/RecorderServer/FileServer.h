#pragma once
#include <string>
#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <thread>
#include "BlockingQueue.h"
#include <memory>
#include <unordered_map>
#include "FileOverLappedContext.h"
#include <mutex>
#include <string>
#include <chrono>
#include <atomic>
class FileServer
{
private:
	struct packet {
		std::vector<char> buffer;
		std::string srcIp;
		uint16_t srcPort;
		std::string dstIp;
		uint16_t dstPort;
		std::chrono::time_point<std::chrono::system_clock> rxTimeSec;
	};

	struct fileInfo {
		HANDLE fileHandle;
		HANDLE IOPort;
		std::string key;
		std::unordered_map<uint64_t, std::shared_ptr<FileOverLappedContext>> ctxList;
		std::shared_ptr<std::mutex> ctxMutex = std::make_shared<std::mutex>();
	};

	const std::wstring _workDir;
	std::vector<std::thread> _receivedPacketWorkers;
	std::vector<std::thread> _fileWriterWorkers;

	bool _terminate{false};
	BlockingQueue<std::shared_ptr<packet>> _queue;
	HANDLE _completionPort{ NULL};
	std::atomic_uint64_t _keyCounter{0};
	
	std::unordered_map<std::string, std::shared_ptr<fileInfo>> _fileHandleList;

	std::mutex _fileMutex;

	void ReceivedPacketWorker();
	void FileWriterWorker();
	std::shared_ptr<FileServer::fileInfo> OpenFile(const std::shared_ptr<FileServer::packet> &ppacket);

	void SetPCapBuffer(std::vector<char> &buffer, const std::shared_ptr<FileServer::packet> &packet);
	void CreatePcapFile(const std::wstring &fileName);
public:
	FileServer(const std::wstring &workDir);
	~FileServer();
	void StartServer(uint8_t numberOfThreads);
	void StopServer();

	void SaveData(const WSABUF &buffer, DWORD receivedBytes, const sockaddr_in  &from, const std::string &dstIp, int16_t dstPort);
};

