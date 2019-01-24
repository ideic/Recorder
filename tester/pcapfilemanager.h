#pragma once
#include <string>
#include <fstream>
#include <vector>
#include "pcap.h"


class PCapFileManager
{
	std::ifstream pcapFile;

	void openFile(const std::string &fileName);

public:
	PCapFileManager(const std::string &fileName);
	~PCapFileManager();


	bool pcap_next_ex(pcap_pkthdr &header, std::vector<uint8_t> &pkt_data);

//	void DumpFile(const std::string &filename, std::vector<PcapPackage>& pcapbuffer);
};
