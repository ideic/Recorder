#include "stdafx.h"
#include "FileServer.h"
#include <algorithm>
#include "ws2tcpip.h"
#include "LoggerFactory.h"
#include <chrono>
#include "PCapPacket.h"
#include <locale> 
#include <codecvt>
#include <fstream>
FileServer::FileServer(std::wstring workDir): _workDir(workDir), _terminate(false)
{
}

FileServer::~FileServer()
{
	CloseHandle(_completionPort);
}

void FileServer::StartServer(uint8_t numberOfThreads)
{
	_terminate = false;

	_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	if (!_completionPort) {
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
			_completionPort,
			&numberOfBytes,
			&completionKey,
			&ctx,
			1000 //  dwMilliseconds
		);

		if (ioSucceeds) {
			if (&numberOfBytes > 0) {

				auto overlappedContext = (FileOverLappedContext*)ctx;
				{
					std::lock_guard<std::mutex> _ctxLock(_ctxMutex);
					_ctxList.erase(overlappedContext->Key);
				}
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

		auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(packet.rxTimeSec.time_since_epoch()).count();
		ctx->Key = packet.srcIp + std::to_string(packet.srcPort) + std::to_string(nanosec);
		//ctx->buffer = packet.buffer;

		{
			std::lock_guard<std::mutex> _ctxLock(_ctxMutex);
			//while (_ctxList.find(ctx->Key) != _ctxList.end()) {
			//	ctx->Key = ctx->Key + 10000;
			//}

			_ctxList.emplace(ctx->Key, ctx);
		}

		SetPCapBuffer(ctx->buffer, packet);

		WriteFile(fileInfo.fileHandle, ctx->buffer.data(), static_cast<DWORD>(ctx->buffer.size()), NULL, ctx.get());
	}

	LoggerFactory::Logger()->LogInfo("Stop FileServer Close Files");

	for (auto& fileHandle : _fileHandleList) {
		CloseHandle(fileHandle.second.fileHandle);
		//CloseHandle(fileHandle.second.IOPort);
	}
}

void FileServer::SetPCapBuffer(std::vector<char> &buffer, FileServer::packet &packet) {
	UDPPacket udp(packet.buffer.size()+100);

	auto dstIpRaw = ntohl(inet_addr(packet.dstIp.c_str()));

	udp.SetDstAddr(dstIpRaw);
	udp.SetDstPort(packet.dstPort);
	
	udp.SetSrcAddr(ntohl(inet_addr(packet.srcIp.c_str())));
	udp.SetSrcPort(packet.srcPort);
	
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(packet.rxTimeSec.time_since_epoch()).count();
	auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(packet.rxTimeSec.time_since_epoch()).count() % 1000000;
	udp.SetTimeH(sec);
	udp.SetTimeL(microsec);
	
	memcpy(udp.GetPayload(), packet.buffer.data(), packet.buffer.size());
	udp.SetPayloadSize(packet.buffer.size());

	char *p = 0;
	unsigned int pCapSize;
	udp.GetPcapBuffer(&p, pCapSize);

	buffer.clear();
	buffer.insert(buffer.begin(), p, p + pCapSize);

}

void FileServer::CreatePcapFile(std::wstring fileName)
{
	struct pcap_file_header {
		uint32_t magic;
		u_short version_major;
		u_short version_minor;
		int32_t thiszone;	/* gmt to local correction */
		uint32_t sigfigs;	/* accuracy of timestamps */
		uint32_t snaplen;	/* max length saved portion of each pkt */
		uint32_t linktype;	/* data link type (LINKTYPE_*) */
	};
	std::ofstream pcapFile;
	pcapFile.open(fileName, std::ofstream::binary | std::ofstream::out);

	if (pcapFile.fail()) {
		LoggerFactory::Logger()->LogWarning(L"Cannot Create file:" + fileName);
		return;
	}

	pcap_file_header fileheader;

	fileheader.version_major = 2;
	fileheader.version_minor = 4;
	fileheader.magic = 0xa1b23c4d;
	fileheader.thiszone = 0;
	fileheader.linktype = 1;
	fileheader.sigfigs = 0;
	fileheader.snaplen = 65535;

	pcapFile.write(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));
	pcapFile.close();
}

FileServer::fileInfo FileServer::OpenFile(FileServer::packet ppacket) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

	std::wstring fileName = _workDir + converter.from_bytes(ppacket.srcIp)  + L"." +std::to_wstring(ppacket.srcPort) + L".pcap";
	
	std::lock_guard<std::mutex> guard(_fileMutex);

	auto fH = _fileHandleList.find(fileName);
	if ((fH) != _fileHandleList.end()) {
		return fH->second;
	}

	FileServer::fileInfo result;

	result.fileHandle = CreateFile(fileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (result.fileHandle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			CreatePcapFile(fileName);
			result.fileHandle = CreateFile(fileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);//CreateFile(fileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
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
	
	result.IOPort = CreateIoCompletionPort(result.fileHandle, _completionPort, (int16_t)ppacket.srcPort, 0);

	if (!result.IOPort) {
		int error = GetLastError();
		LoggerFactory::Logger()->LogWarning("Cannot Create file IOPort:" + error);
	}

	_fileHandleList[fileName] = result;
	return result;
}

void FileServer::SaveData(WSABUF buffer, DWORD receivedBytes, sockaddr_in from, std::string dstIp, int16_t dstPort)
{
	//wchar_t ip[INET_ADDRSTRLEN];
	//InetNtop(from.sin_family, &from.sin_addr, ip, INET_ADDRSTRLEN);

	packet p;
	p.buffer.clear();
	p.buffer.insert(p.buffer.end(), buffer.buf, buffer.buf + receivedBytes);
	p.srcIp = inet_ntoa(((sockaddr_in)from).sin_addr);
	p.srcPort = from.sin_port;
	
	p.dstIp = dstIp;
	p.dstPort = dstPort;

	p.rxTimeSec = std::chrono::system_clock::now();

	_queue.push(std::move(p));
}
