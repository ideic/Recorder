#include "cmfxhandlerstore.h"
#include <algorithm>
#include "asyncudpsocket.h"
#include "cmfxfilehandler.h"

using namespace std;


CmfxHandlerStore::CmfxHandlerStore()
{
}

CmfxHandlerStore::~CmfxHandlerStore()
{
}

void CmfxHandlerStore::add(shared_ptr<CmfxFile> cmfxFile, AsyncUdpSocketFactory& asyncUdpSocketFactory) {
	cmfxFileHandlers.push_back(unique_ptr<CmfxFileHandler>(new CmfxFileHandler(cmfxFile, asyncUdpSocketFactory)));
	packetListWithSockets.push_back(cmfxFileHandlers.back()->getNextPacketListWithSocket());
}

bool packetListComp(const shared_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>>& a, 
	const shared_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>>& b) {
	if (nullptr == a || nullptr == b) {
		if (nullptr == a && nullptr == b) {
			return a < b;
		}

		if (nullptr == b) {
			return true;
		}

		return false;
	}

	return a->second->timeStamp < b->second->timeStamp;
}

shared_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> CmfxHandlerStore::getNextPacketListWithSocket() {
	shared_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> result;

	if (packetListWithSockets.empty()) {
		return result;
	}

	auto it = min_element(packetListWithSockets.begin(), packetListWithSockets.end(), packetListComp);

	if ((packetListWithSockets.end() != it) && (nullptr != *it)) {
		result = *it;
		const size_t index = it - packetListWithSockets.begin();
		packetListWithSockets[index] = cmfxFileHandlers[index]->getNextPacketListWithSocket();
	}

	return result;
}