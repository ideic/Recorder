#pragma once
#include <memory>
#include <vector>
#include "udp.h"

class AsyncUdpSocket;
class AsyncUdpSocketFactory;
class CmfxFile;


class CmfxFileHandler
{
	const std::shared_ptr<CmfxFile> cmfxFile;
	std::vector<std::shared_ptr<AsyncUdpSocket>> asyncUdpSockets;
	size_t packetIndex;

public:
	CmfxFileHandler(std::shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory);
	~CmfxFileHandler();

	std::unique_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> getNextPacketListWithSocket();
};

