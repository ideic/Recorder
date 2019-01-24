#pragma once
#include <chrono>
#include <string>
#include <vector>
#include "udp.h"


typedef struct {
	std::vector<UdpPacketDataPtr> udpPacketList;
	std::chrono::microseconds timeStamp;
} UdpPacketDataListWithTimeStamp;


class Stream {
	std::vector<UdpPacketDataListWithTimeStamp> udpPacketDatas;

	void processFile(const std::string& fileName);
	UdpPacketDataPtr getUdpPacketData(const std::vector<uint8_t>& pcapData);

public:
	Stream(const std::string& fileName);
	~Stream();

	const std::vector<UdpPacketDataListWithTimeStamp>& getUdpPacketDatas() const;
};
