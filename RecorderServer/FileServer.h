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
class FileServer
{
private:
	struct packet {
		std::vector<char> buffer;
		std::wstring from;
		uint16_t port;
	};

	struct fileInfo {
		HANDLE fileHandle;
		HANDLE IOPort;
	};

	std::wstring _workDir;
	std::vector<std::thread> _receivedPacketWorkers;
	std::vector<std::thread> _fileWriterWorkers;

	bool _terminate;
	BlockingQueue<packet> _queue;
	HANDLE _completionPort{ NULL};
	std::unordered_map<int32_t, std::shared_ptr<FileOverLappedContext>> _ctxList;
	std::unordered_map<std::wstring, fileInfo> _fileHandleList;

	std::mutex _fileMutex;

	void ReceivedPacketWorker();
	void FileWriterWorker();
	FileServer::fileInfo OpenFile(FileServer::packet ppacket);
public:
	FileServer(std::wstring workDir);
	~FileServer();
	void StartServer(uint8_t numberOfThreads);
	void StopServer();

	void SaveData(WSABUF buffer, DWORD receivedBytes, sockaddr_in  from);
};

