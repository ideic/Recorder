#pragma once
#include <chrono>
#include <memory>
#include <vector>


typedef struct {
	size_t length;
	std::unique_ptr<uint8_t> buffer;
} UdpPacketData;

typedef std::shared_ptr<UdpPacketData> UdpPacketDataPtr;


typedef struct {
	std::vector<UdpPacketDataPtr> udpPacketList;
	std::chrono::microseconds timeStamp;
} UdpPacketDataListWithTimeStamp;
