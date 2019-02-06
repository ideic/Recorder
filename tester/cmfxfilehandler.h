#pragma once
#include <memory>
#include <vector>
#include "udp.h"

class AsyncUdpSocket;
class AsyncUdpSocketFactory;
class Stream;


class CmfxFileHandler {
	const size_t sendTimes;
	AsyncUdpSocketFactory& asyncUdpSocketFactory;

	std::vector<std::shared_ptr<Stream>> streams;
	std::vector<std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> sortedUdpPacketDataLists;
	std::vector<std::shared_ptr<AsyncUdpSocket>> asyncUdpSockets;

	size_t currentPacketIndex;
	size_t currentSendTime;

public:
	CmfxFileHandler(AsyncUdpSocketFactory& asyncUdpSocketFactory, size_t sendTimes = 1);
	~CmfxFileHandler();

	void addCmfxFile(const std::string fileName);
	void mergeFiles();
	size_t getStreamCount() const { return streams.size();  }

	std::unique_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> getNextPacketListWithSocket();
};

