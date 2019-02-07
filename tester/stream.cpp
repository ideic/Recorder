#include "stream.h"

using namespace std;


Stream::Stream() {
}

Stream::~Stream() {
}

void Stream::addUdpPacket(const chrono::microseconds& timeStamp, UdpPacketDataPtr udpPacket) {
	if (!udpPacketDataLists.empty() && (timeStamp < udpPacketDataLists.back()->timeStamp)) {
		throw runtime_error("Time stamp error");
	}

	if (udpPacketDataLists.empty() || (udpPacketDataLists.back()->timeStamp != timeStamp)) {
		udpPacketDataLists.push_back(shared_ptr<UdpPacketDataListWithTimeStamp>(new UdpPacketDataListWithTimeStamp()));
		udpPacketDataLists.back()->timeStamp = timeStamp;
	}

	udpPacketDataLists.back()->udpPacketList.push_back(udpPacket);
}
