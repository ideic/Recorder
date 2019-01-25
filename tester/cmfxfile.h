#pragma once
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "header.h"
#include "udp.h"


class AsyncUdpSocket;
class AsyncUdpSocketFactory;
class Stream;

class CmfxFile
{
	std::vector<std::shared_ptr<Stream>> streams;

	void processFile(const std::string& fileName);

	static UdpPacketDataPtr getUdpPacketData(const udp_header* udpHeader);
	static udp_header* toUdpHeader(const std::vector<uint8_t>& pcapData);

public:
	CmfxFile(const std::string& fileName);
	~CmfxFile();

	const std::vector<std::shared_ptr<Stream>>& getStreams() const { return streams; }
};

