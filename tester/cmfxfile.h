#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "header.h"
#include "udp.h"

class Stream;


class CmfxFile {
	std::vector<std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> udpPacketDataLists;
	size_t streamCount;

	std::map<u_short, std::shared_ptr<Stream>> processFile(const std::string& fileName);

	static UdpPacketDataPtr getUdpPacketData(const udp_header* udpHeader);
	static udp_header* toUdpHeader(const std::vector<uint8_t>& pcapData);

public:
	CmfxFile(const std::string& fileName);
	~CmfxFile();

	const std::vector<std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>>& getUdpPacketDataLists() const { return udpPacketDataLists; }
	const size_t getStreamCount() const { return streamCount;  }
};

