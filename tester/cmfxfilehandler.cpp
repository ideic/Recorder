#include "cmfxfilehandler.h"
#include <chrono>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "stream.h"

using namespace std;


CmfxFileHandler::CmfxFileHandler(shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory) :
	cmfxFile(cmfxFile),
	asyncUdpSockets(cmfxFile->getStreamCount()),
	packetIndex(0)
{
	for (size_t i = 0; i < asyncUdpSockets.size(); ++i) {
		asyncUdpSockets[i] = asyncUdpSocketFactory.create();
	}
}

CmfxFileHandler::~CmfxFileHandler() {
}

unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> CmfxFileHandler::getNextPacketListWithSocket() {
	unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> result;

	if (packetIndex < cmfxFile->getUdpPacketDataLists().size()) {
		result.reset(new pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>(shared_ptr<AsyncUdpSocket>(), shared_ptr<UdpPacketDataListWithTimeStamp>()));

		const std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>& udpPacketDataList = cmfxFile->getUdpPacketDataLists().at(packetIndex);
		const size_t socketIndex = udpPacketDataList.first;

		if (socketIndex >= asyncUdpSockets.size()) {
			throw logic_error("CmfxFileHandler::getNextPacketListWithSocket()  socketIndex >= asyncUdpSockets.size()");
		}

		result->first = asyncUdpSockets[socketIndex];
		result->second = udpPacketDataList.second;
		packetIndex++;
	}

	return result;
}
