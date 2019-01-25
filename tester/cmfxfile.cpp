#include "cmfxfile.h"
#include <chrono>
#include <map>
#include "asyncudpsocket.h"
#include "pcapfilemanager.h"
#include "stream.h"

using namespace std;


CmfxFile::CmfxFile(const string& fileName) {
	processFile(fileName);
}

CmfxFile::~CmfxFile() {
}

void CmfxFile::processFile(const string& fileName) {
	PCapFileManager pcap(fileName);
	pcap_pkthdr pcapHeader;
	vector<uint8_t> pcapData;

	map<u_short, shared_ptr<Stream>> streamMap;

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

		auto streamMapIt = streamMap.find(udpHeader->dport);
		if (streamMap.end() == streamMapIt) {
			shared_ptr<Stream> newStream(new Stream());
			streamMapIt = streamMap.insert(make_pair(udpHeader->dport, newStream)).first;
			streams.push_back(newStream);
		}

		shared_ptr<Stream> stream = streamMapIt->second;
		stream->addUdpPacket(timeStamp, getUdpPacketData(udpHeader));
	}
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
