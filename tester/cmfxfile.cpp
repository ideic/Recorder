#include "cmfxfile.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include "pcapfilemanager.h"
#include "stream.h"

using namespace std;


CmfxFile::CmfxFile(const string& fileName) :
	streamCount(0)
{
	cout << fileName << ": " << endl;

	map<u_short, shared_ptr<Stream>> streams = processFile(fileName);

	cout << streams.size() << " streams found" << endl;

	for (auto it = streams.begin(); streams.end() != it; ++it) {
		std::shared_ptr<Stream> stream = it->second;

		udpPacketDataLists.reserve(udpPacketDataLists.size() + stream->getUdpPacketDataLists().size());
		for (size_t packetIndex = 0; packetIndex < stream->getUdpPacketDataLists().size(); ++packetIndex) {
			udpPacketDataLists.push_back(make_pair(streamCount, stream->getUdpPacketDataLists().at(packetIndex)));
		}

		streamCount++;
	}

	auto sorter = [](
		const std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>& a,
		const std::pair<size_t, std::shared_ptr<UdpPacketDataListWithTimeStamp>>& b)
	{
		return (a.second->timeStamp < b.second->timeStamp);
	};

	sort(udpPacketDataLists.begin(), udpPacketDataLists.end(), sorter);
}

CmfxFile::~CmfxFile() {
}

map<u_short, shared_ptr<Stream>> CmfxFile::processFile(const string& fileName) {
	PCapFileManager pcap(fileName);
	pcap_pkthdr pcapHeader;
	vector<uint8_t> pcapData;

	map<u_short, shared_ptr<Stream>> streams;

	// First packet is the pcap file header, drop it
	pcap.pcap_next_ex(pcapHeader, pcapData);

	while (pcap.pcap_next_ex(pcapHeader, pcapData)) {
		/*
		cout << "packet ts: " << pcapHeader.ts.tv_sec << "." << dec << pcapHeader.ts.tv_usec << " ";
		cout << "caplen: " << pcapHeader.caplen << " ";
		cout << "len: " << pcapHeader.len << endl;
		*/
		const chrono::microseconds timeStamp = chrono::seconds(pcapHeader.ts.tv_sec) + chrono::microseconds(pcapHeader.ts.tv_usec);
		const udp_header* udpHeader = toUdpHeader(pcapData);

		auto it = streams.find(udpHeader->dport);
		if (streams.end() == it) {
			shared_ptr<Stream> newStream(new Stream());
			it = streams.insert(make_pair(udpHeader->dport, newStream)).first;
		}

		shared_ptr<Stream> stream = it->second;
		stream->addUdpPacket(timeStamp, getUdpPacketData(udpHeader));
	}

	return streams;
}

udp_header* CmfxFile::toUdpHeader(const vector<uint8_t>& pcapData) {
	ip_header *ipHeader = (ip_header *)(pcapData.data() + sizeof(ethernet_header));

	if (IPPROTO_UDP != ipHeader->proto) {
		throw runtime_error("Not UDP packet!!!");
	}

	u_int ipLength = (ipHeader->ver_ihl & 0xf) * 4;
	udp_header *udpHeader = (udp_header *)((u_char*)ipHeader + ipLength);
	return udpHeader;
}

UdpPacketDataPtr CmfxFile::getUdpPacketData(const udp_header* udpHeader) {
	u_char* udpData = (u_char*)udpHeader + sizeof(udp_header);
	u_int udpDataLength = ntohs(udpHeader->len) - sizeof(udp_header);

	UdpPacketDataPtr result(new UdpPacketData());
	result->length = udpDataLength;
	result->buffer.reset(new uint8_t[result->length]);
	memcpy_s(result->buffer.get(), result->length, udpData, udpDataLength);
	return result;
}
