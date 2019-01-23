#include "stdafx.h"
#include "FileServer.h"
#include <algorithm>
#include "ws2tcpip.h"
#include "LoggerFactory.h"

FileServer::FileServer(std::wstring workDir): _workDir(workDir), _terminate(false)
{
}

FileServer::~FileServer()
{
}

void FileServer::StartServer(uint8_t numberOfThreads)
{
	_terminate = false;

	_completionPort.reset(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0));

	if (_completionPort.get() == NULL) {
		throw std::runtime_error("IO Completion port create failed at FileServer with error: " + GetLastError());
	}

	LoggerFactory::Logger()->LogInfo("File Server starts receivedPacketWorkers with threads:" + std::to_string(numberOfThreads));

	for (int i = 0; i < numberOfThreads; ++i) {
		_receivedPacketWorkers.emplace_back([&]() {ReceivedPacketWorker(); });
	}

	LoggerFactory::Logger()->LogInfo("File Server starts FileWritertWorkers with threads:" + std::to_string(numberOfThreads));

	for (int i = 0; i < numberOfThreads; ++i) {
		_fileWriterWorkers.emplace_back([&]() {FileWriterWorker(); });
	}
}

void FileServer::StopServer() {
	_terminate = true;

	LoggerFactory::Logger()->LogInfo("Stop FileServer ReceivedPacketWorkers");


	//wake up all the threads, which are locked on queue
	for (auto& worker : _receivedPacketWorkers) {
		packet p;
		_queue.push(std::move(p));
	};

	for (auto& worker : _receivedPacketWorkers) {
		worker.join();
	};

	LoggerFactory::Logger()->LogInfo("Stop FileServer FileWriteWorkers");

	for (auto& worker : _fileWriterWorkers) {
		worker.join();
	};

	_receivedPacketWorkers.clear();
	_fileWriterWorkers.clear();
}

void FileServer::FileWriterWorker() {
	while (!_terminate) {
		DWORD numberOfBytes = 0;
		unsigned long long completionKey = 0;

		LPOVERLAPPED ctx = 0;

		auto ioSucceeds = GetQueuedCompletionStatus(
			_completionPort.get(),
			&numberOfBytes,
			&completionKey,
			&ctx,
			1000 //  dwMilliseconds
		);

		if (ioSucceeds) {
			if (&numberOfBytes > 0) {

				auto overlappedContext = (FileOverLappedContext*)ctx;
				_ctxList.erase(overlappedContext->Key);
			}
		}
	}
}

void FileServer::ReceivedPacketWorker()
{
	while (!_terminate) {
		auto packet = _queue.getNext();

		if (packet.buffer.size() == 0) continue;

		auto fileInfo = OpenFile(packet);
		if (fileInfo.fileHandle == INVALID_HANDLE_VALUE || !fileInfo.IOPort) continue;

		std::shared_ptr<FileOverLappedContext> ctx = std::make_shared<FileOverLappedContext>();
		ctx->FileHandle = fileInfo.fileHandle;
		ctx->IOPort = fileInfo.IOPort;
		ctx->Key = packet.port;
		ctx->buffer = packet.buffer;

		while (_ctxList.find(ctx->Key) != _ctxList.end()) {
			ctx->Key = ctx->Key + 10000;
		}
		_ctxList.emplace(packet.port, ctx);
		WriteFile(fileInfo.fileHandle, ctx->buffer.data(), static_cast<DWORD>(ctx->buffer.size()), NULL, ctx.get());
	}

	LoggerFactory::Logger()->LogInfo("Stop FileServer Close Files");

	for (auto& fileHandle : _fileHandleList) {
		CloseHandle(fileHandle.second.fileHandle);
		CloseHandle(fileHandle.second.IOPort);
	}
}

FileServer::fileInfo FileServer::OpenFile(FileServer::packet ppacket) {
	std::wstring fileName = _workDir + ppacket.from + L"." +std::to_wstring(ppacket.port) + L".raw";
	
	std::lock_guard<std::mutex> guard(_fileMutex);

	auto fH = _fileHandleList.find(fileName);
	if ((fH) != _fileHandleList.end()) {
		return fH->second;
	}

	FileServer::fileInfo result;

	result.fileHandle = CreateFile(fileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (result.fileHandle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			result.fileHandle = CreateFile(fileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
			if (result.fileHandle == INVALID_HANDLE_VALUE) {
				LoggerFactory::Logger()->LogWarning(L"Cannot Create file:" + fileName);
				return result;
			}
		}
		else {
			LoggerFactory::Logger()->LogWarning(L"Cannot Create file for append:" + fileName);

			return result;
		}
	}
	
	result.IOPort = CreateIoCompletionPort(result.fileHandle, _completionPort.get(), (int16_t)ppacket.port, 0);

	if (!result.IOPort) {
		int error = GetLastError();
		LoggerFactory::Logger()->LogWarning("Cannot Create file IOPort:" + error);
	}

	_fileHandleList[fileName] = result;
	return result;
}

void FileServer::SaveData(WSABUF buffer, DWORD receivedBytes, sockaddr_in from)
{
	wchar_t ip[INET_ADDRSTRLEN];
	InetNtop(from.sin_family, &from.sin_addr, ip, INET_ADDRSTRLEN);

	packet p;
	p.buffer.clear();
	p.buffer.insert(p.buffer.end(), buffer.buf, buffer.buf + receivedBytes);
	p.from = std::wstring(ip);
	p.port = from.sin_port;

	_queue.push(std::move(p));
}
