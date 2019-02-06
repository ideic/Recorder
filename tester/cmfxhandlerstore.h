#pragma once
#include <memory>
#include <vector>
#include "cmfxfilehandler.h"

class AsyncUdpSocket;
class AsyncUdpSocketFactory;
class CmfxFile;
class CmfxFileHandler;


class CmfxHandlerStore
{
	std::vector<std::unique_ptr<CmfxFileHandler>> cmfxFileHandlers;
	std::vector<std::shared_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>>> packetListWithSockets;

public:
	CmfxHandlerStore();
	~CmfxHandlerStore();

	void add(std::shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory);
	std::shared_ptr<std::pair<std::shared_ptr<AsyncUdpSocket>, std::shared_ptr<UdpPacketDataListWithTimeStamp>>> getNextPacketListWithSocket();
};

