#include "cmfxfilehandler.h"
#include <chrono>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "stream.h"

using namespace std;


CmfxFileHandler::CmfxFileHandler(shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory) :
	streamCount(cmfxFile->getStreams().size()),
	cmfxFile(cmfxFile),
	asyncUdpSockets(streamCount),
	packetIndexes(streamCount)
{
	for (size_t i = 0; i < streamCount; ++i) {
		asyncUdpSockets[i] = asyncUdpSocketFactory.create(i);
	}

	resetPosition();
}

CmfxFileHandler::~CmfxFileHandler()
{
}

void CmfxFileHandler::resetPosition() {
	for (size_t i = 0; i < packetIndexes.size(); ++i) {
		packetIndexes[i] = 0;
	}
}

unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> CmfxFileHandler::getNextPackets() {
	auto maxFunc = chrono::microseconds::max;
	chrono::microseconds minTimestamp(maxFunc());
	size_t minStreamIndex = streamCount;

	for (size_t streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
		shared_ptr<Stream> stream = cmfxFile->getStreams().at(streamIndex);
		size_t packetIndex = packetIndexes[streamIndex];

		if (packetIndex < stream->getUdpPacketDatas().size()) {
			if (minTimestamp > stream->getUdpPacketDatas().at(packetIndex)->timeStamp) {
				minTimestamp = stream->getUdpPacketDatas().at(packetIndex)->timeStamp;
				minStreamIndex = streamIndex;
			}
		}
	}

	unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> result;

	if (minStreamIndex < streamCount) {
		shared_ptr<AsyncUdpSocket> asyncUdpSocket = asyncUdpSockets[minStreamIndex];
		shared_ptr<Stream> stream = cmfxFile->getStreams().at(minStreamIndex);
		size_t packetIndex = packetIndexes[minStreamIndex];
		shared_ptr<UdpPacketDataListWithTimeStamp> udpPacketDataList = stream->getUdpPacketDatas().at(packetIndex);
		result.reset(new pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>(asyncUdpSocket, udpPacketDataList));

		packetIndexes[minStreamIndex] += 1;
	}

	return result;
}