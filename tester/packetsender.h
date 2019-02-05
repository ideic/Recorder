#pragma once
#include <memory>
#include <vector>
#include "blockingqueue.h"
#include "udp.h"


class AsyncUdpSocket;

typedef BlockingQueue<std::unique_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>>> BlockingQueueUdpPacket;


class PacketSender {
	BlockingQueueUdpPacket& blockingQueue;
	std::vector<std::thread> workerThreads;
	const std::chrono::time_point<std::chrono::steady_clock> startTime;


	void workerFunc();

public:
	PacketSender(BlockingQueueUdpPacket& blockingQueue, size_t threadCount, const std::chrono::time_point<std::chrono::steady_clock>& startTime);
	~PacketSender();

	void join();
};

