#include "packetsender.h"
#include "asyncudpsocket.h"
#include "udp.h"

using namespace std;


PacketSender::PacketSender(BlockingQueueUdpPacket& blockingQueue, size_t threadCount, const std::chrono::time_point<std::chrono::steady_clock>& startTime) :
	blockingQueue(blockingQueue),
	workerThreads(threadCount),
	startTime(startTime)
{
	for (size_t i = 0; i < workerThreads.size(); ++i) {
		workerThreads[i] = thread(&PacketSender::workerFunc, this);
	}
}

PacketSender::~PacketSender() {
}

void PacketSender::join() {
	for (auto& workerThread : workerThreads) {
		workerThread.join();
	}
}

void PacketSender::workerFunc() {
	while (blockingQueue.hasNext()) {
		std::unique_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> packetListWithSocket = blockingQueue.getNext();
		shared_ptr<AsyncUdpSocket> asyncUdpSocket = packetListWithSocket->first;
		shared_ptr<UdpPacketDataListWithTimeStamp> udpPacketDataList = packetListWithSocket->second;

		this_thread::sleep_until(startTime + udpPacketDataList->timeStamp);

		for (size_t i = 0; i < udpPacketDataList->udpPacketList.size(); ++i) {
			asyncUdpSocket->sendUdpPacket(udpPacketDataList->udpPacketList[i]);
		}
	}
}