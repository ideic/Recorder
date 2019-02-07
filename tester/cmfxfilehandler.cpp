#include "cmfxfilehandler.h"
#include <algorithm>
#include <chrono>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "stream.h"

using namespace std;


CmfxFileHandler::CmfxFileHandler(AsyncUdpSocketFactory& asyncUdpSocketFactory, size_t sendTimes) :
	sendTimes(sendTimes),
	asyncUdpSocketFactory(asyncUdpSocketFactory),
	currentPacketIndex(0),
	currentSendTime(0)
{
}

CmfxFileHandler::~CmfxFileHandler() {
}

void CmfxFileHandler::addCmfxFile(const string fileName) {
	CmfxFile cmfxFile(fileName);

	for (auto stream : cmfxFile.getStreams()) {
		streams.push_back(stream);
	}
}

void CmfxFileHandler::mergeFiles() {

	asyncUdpSockets.resize(streams.size() * sendTimes);
	for (size_t i = 0; i < asyncUdpSockets.size(); ++i) {
		asyncUdpSockets[i] = asyncUdpSocketFactory.create();
	}

	for (size_t streamIdx = 0; streamIdx < streams.size(); ++streamIdx) {
		shared_ptr<Stream> stream = streams[streamIdx];

		sortedUdpPacketDataLists.reserve(sortedUdpPacketDataLists.size() + stream->getUdpPacketDataLists().size());
		for (size_t packetIndex = 0; packetIndex < stream->getUdpPacketDataLists().size(); ++packetIndex) {
			sortedUdpPacketDataLists.push_back(make_pair(streamIdx, stream->getUdpPacketDataLists().at(packetIndex)));
		}
	}

	auto sorter = [](
		const pair<size_t, shared_ptr<UdpPacketDataListWithTimeStamp>>& a,
		const pair<size_t, shared_ptr<UdpPacketDataListWithTimeStamp>>& b)
	{
		return (a.second->timeStamp < b.second->timeStamp);
	};

	sort(sortedUdpPacketDataLists.begin(), sortedUdpPacketDataLists.end(), sorter);
}

unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> CmfxFileHandler::getNextPacketListWithSocket() {
	unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> result;

	if (currentPacketIndex < sortedUdpPacketDataLists.size()) {
		result.reset(new pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>(shared_ptr<AsyncUdpSocket>(), shared_ptr<UdpPacketDataListWithTimeStamp>()));

		const pair<size_t, shared_ptr<UdpPacketDataListWithTimeStamp>>& udpPacketDataList = sortedUdpPacketDataLists[currentPacketIndex];
		const size_t socketIndex = udpPacketDataList.first + getStreamCount() * currentSendTime;

		if (socketIndex >= asyncUdpSockets.size()) {
			throw logic_error("CmfxFileHandler::getNextPacketListWithSocket()  socketIndex >= asyncUdpSockets.size()");
		}

		result->first = asyncUdpSockets[socketIndex];
		result->second = udpPacketDataList.second;

		currentSendTime++;
		if (currentSendTime >= sendTimes) {
			currentPacketIndex++;
			currentSendTime = 0;
		}
	}

	return result;
}
