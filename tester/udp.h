#pragma once
#include <memory>


typedef struct {
	size_t length;
	std::unique_ptr<uint8_t> buffer;
} UdpPacketData;

typedef std::shared_ptr<UdpPacketData> UdpPacketDataPtr;
