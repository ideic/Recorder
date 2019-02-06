#include "cmfxfile.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include "pcapfilemanager.h"
#include "stream.h"

using namespace std;


CmfxFile::CmfxFile(const string& fileName) {
	cout << fileName << ": " << endl;

	map<u_short, shared_ptr<Stream>> streamMap = processFile(fileName);
	for (auto keyValue : streamMap) {
		streams.push_back(keyValue.second);
	}

	shiftStreamsStartToZero();
}

CmfxFile::~CmfxFile() {
}

void CmfxFile::shiftStreamsStartToZero() {
	struct TimestampComp {
		bool operator()(const shared_ptr<Stream>& a, const shared_ptr<Stream>& b) {
			return (a->getUdpPacketDataLists().front()->timeStamp < b->getUdpPacketDataLists().front()->timeStamp);
		}
	};

	struct DecrementTimestamp {
		chrono::microseconds deltaT;

		DecrementTimestamp(chrono::microseconds deltaT) : deltaT(deltaT) {
		}

		void operator()(shared_ptr<UdpPacketDataListWithTimeStamp> udpPacketDataListWithTimeStamp) {
			udpPacketDataListWithTimeStamp->timeStamp -= deltaT;
		}
	};

	auto minTimestampIt = min_element(streams.begin(), streams.end(), TimestampComp());

	if (streams.end() != minTimestampIt) {
		auto minTimestamp = (*minTimestampIt)->getUdpPacketDataLists().front()->timeStamp;

		for (auto stream : streams) {
			for_each(stream->getUdpPacketDataLists().begin(), stream->getUdpPacketDataLists().end(), DecrementTimestamp(minTimestamp));
		}
	}
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
