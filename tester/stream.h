#pragma once
#include <chrono>
#include <string>
#include <vector>
#include "header.h"
#include "udp.h"


struct UdpPacketDataListWithTimeStamp {
	std::vector<UdpPacketDataPtr> udpPacketList;
	std::chrono::microseconds timeStamp;
};


class Stream {
	std::vector<std::shared_ptr<UdpPacketDataListWithTimeStamp>> udpPacketDatas;

public:
	Stream();
	~Stream();

	void addUdpPacket(const std::chrono::microseconds& timeStamp, UdpPacketDataPtr udpPacket);

	const std::vector<std::shared_ptr<UdpPacketDataListWithTimeStamp>>& getUdpPacketDatas() const { return udpPacketDatas; }
};
