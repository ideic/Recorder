#include "stream.h"
#include "pcapfilemanager.h"
#include "pcap.h"
#include <algorithm>
#include <iostream>
#include <map>

using namespace std;


Stream::Stream() {
}

Stream::~Stream() {
}

void Stream::addUdpPacket(const std::chrono::microseconds& timeStamp, UdpPacketDataPtr udpPacket) {
	if (!udpPacketDatas.empty() && (timeStamp < udpPacketDatas.back()->timeStamp)) {
		throw runtime_error("Time stamp error");
	}

	if (udpPacketDatas.empty() || (udpPacketDatas.back()->timeStamp != timeStamp)) {
		udpPacketDatas.push_back(shared_ptr<UdpPacketDataListWithTimeStamp>(new UdpPacketDataListWithTimeStamp()));
		udpPacketDatas.back()->timeStamp = timeStamp;
	}

	udpPacketDatas.back()->udpPacketList.push_back(udpPacket);
}
