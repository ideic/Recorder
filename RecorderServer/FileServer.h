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
	};

	std::wstring _workDir;
	std::vector<std::thread> _receivedPacketWorkers;
	std::vector<std::thread> _fileWriterWorkers;

	bool _terminate;
	BlockingQueue<std::shared_ptr<packet>> _queue;
	HANDLE _completionPort{ NULL};
	std::atomic_uint64_t _keyCounter;
	std::unordered_map<uint64_t, std::shared_ptr<FileOverLappedContext>> _ctxList;
	std::unordered_map<std::string, fileInfo> _fileHandleList;

	std::mutex _fileMutex;
	std::mutex _ctxMutex;

	void ReceivedPacketWorker();
	void FileWriterWorker();
	FileServer::fileInfo OpenFile(std::shared_ptr<FileServer::packet> ppacket);

	void SetPCapBuffer(std::vector<char> &buffer, std::shared_ptr<FileServer::packet> &packet);
	void CreatePcapFile(std::wstring fileName);
public:
	FileServer(std::wstring workDir);
	~FileServer();
	void StartServer(uint8_t numberOfThreads);
	void StopServer();

	void SaveData(WSABUF buffer, DWORD receivedBytes, sockaddr_in  from, std::string dstIp, int16_t dstPort);
};

