#pragma once
#include <chrono>
#include <vector>
#include "udp.h"


class Stream {
	std::vector<std::shared_ptr<UdpPacketDataListWithTimeStamp>> udpPacketDataLists;

public:
	Stream();
	~Stream();

	void addUdpPacket(const std::chrono::microseconds& timeStamp, UdpPacketDataPtr udpPacket);

	const std::vector<std::shared_ptr<UdpPacketDataListWithTimeStamp>>& getUdpPacketDataLists() const { return udpPacketDataLists; }
};
