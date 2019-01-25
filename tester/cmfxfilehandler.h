#pragma once
#include <memory>
#include <vector>


class AsyncUdpSocket;
class AsyncUdpSocketFactory;
class CmfxFile;
struct UdpPacketDataListWithTimeStamp;


class CmfxFileHandler
{
	const size_t streamCount;
	std::shared_ptr<CmfxFile> cmfxFile;
	std::vector<std::shared_ptr<AsyncUdpSocket>> asyncUdpSockets;
	std::vector<size_t> packetIndexes;

public:
	CmfxFileHandler(std::shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory);
	~CmfxFileHandler();

	void resetPosition();

	std::unique_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> getNextPackets();
};

