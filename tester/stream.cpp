#include "stream.h"
#include "pcapfilemanager.h"
#include "header.h"
#include "pcap.h"
#include <algorithm>
#include <iostream>

using namespace std;


Stream::Stream(const std::string& fileName) {
	processFile(fileName);
}

Stream::~Stream() {
}

UdpPacketDataPtr Stream::getUdpPacketData(const std::vector<uint8_t>& pcapData) {
	ip_header *ipHeader = (ip_header *)(pcapData.data() + sizeof(ethernet_header));

	if (IPPROTO_UDP != ipHeader->proto) {
		throw runtime_error("Not UDP packet!!!");
	}

	u_int ipLength = (ipHeader->ver_ihl & 0xf) * 4;
	udp_header *udpHeader = (udp_header *)((u_char*)ipHeader + ipLength);
	u_char* udpData = (u_char*)udpHeader + sizeof(udp_header);
	u_int udpDataLength = ntohs(udpHeader->len) - sizeof(udp_header);

	shared_ptr<UdpPacketData> result(new UdpPacketData());
	result->length = udpDataLength;
	result->buffer.reset(new uint8_t[result->length]);
	memcpy_s(result->buffer.get(), result->length, udpData, udpDataLength);
	return result;
}

void Stream::processFile(const std::string& fileName)
{
	PCapFileManager pcap(fileName);

	pcap_pkthdr pcapHeader;
	std::vector<uint8_t> pcapData;

	// First packet is the pcap file header, drop it
	pcap.pcap_next_ex(pcapHeader, pcapData);

	while (pcap.pcap_next_ex(pcapHeader, pcapData)) {
/*
		cout << "packet ts: " << pcapHeader.ts.tv_sec << "." << dec << pcapHeader.ts.tv_usec << " ";
		cout << "caplen: " << pcapHeader.caplen << " ";
		cout << "len: " << pcapHeader.len << endl;
*/
		const chrono::microseconds timeStamp = chrono::seconds(pcapHeader.ts.tv_sec) + chrono::microseconds(pcapHeader.ts.tv_usec);

		if (!udpPacketDatas.empty() && (timeStamp < udpPacketDatas.back().timeStamp)) {
			cerr << "Time stamp error" << endl;
			continue;
		}

		if (udpPacketDatas.empty() || (udpPacketDatas.back().timeStamp != timeStamp)) {
			udpPacketDatas.push_back(UdpPacketDataListWithTimeStamp());
			udpPacketDatas.back().timeStamp = timeStamp;
		}

		udpPacketDatas.back().udpPacketList.push_back(getUdpPacketData(pcapData));
		udpPacketDatas.back().timeStamp = timeStamp;
	}
}

const vector<UdpPacketDataListWithTimeStamp>& Stream::getUdpPacketDatas() const {
	return udpPacketDatas;
}
