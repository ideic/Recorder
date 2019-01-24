#include "pcapfilemanager.h"
#include <iomanip>
#include <iostream>
#include <ostream>
#include <stdexcept>

using namespace std;


PCapFileManager::PCapFileManager(const string &fileName)
{
	openFile(fileName);
}

PCapFileManager::~PCapFileManager()
{
	pcapFile.close();
}

void PCapFileManager::openFile(const string &fileName) {
	pcapFile.open(fileName, ifstream::binary | ifstream::in);

	if (pcapFile.fail()) {
		throw runtime_error("Can not open file: " + fileName + " " + strerror(errno));
	}

	pcap_file_header fileheader;
	pcapFile.read(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));

	if (!pcapFile.good()) {
		throw runtime_error("Invalid cmfx file: " + fileName);
	}

	cout << fileName << ": " << endl;
	cout << "PCAP file version: " << to_string(fileheader.version_major) << "." << to_string(fileheader.version_minor) << " ";
	cout << "magic number: " << "0x" << setw(8) << setfill('0') << hex << fileheader.magic;
	cout << endl;
}

bool PCapFileManager::pcap_next_ex(pcap_pkthdr &header, vector<uint8_t> &pkt_data) {
	
	pcapFile.read(reinterpret_cast<char*>(&header), sizeof(header));
	
	if (!pcapFile.good()) {
		return false;
	}

	if (header.caplen == 0) {
		return false;
	}

	pkt_data.resize(header.caplen);
	pcapFile.read(reinterpret_cast<char*>(pkt_data.data()), pkt_data.size());

	if (!pcapFile.good()) {
		return false;
	}
	
	return true;
}

/*
void PCapFileManager::DumpFile(const string &fileName,  vector<PcapPackage>& pcapbuffer)
{
	ofstream dumpFile;
	dumpFile.open(fileName, ofstream::binary | ofstream::out);

	if (dumpFile.fail()) {
		ERRORLOGGING << __THREAD_TIME_FILE_LINE__ + "Pcap file write error" + strerror(errno);
		return;
	}

	pcap_file_header fileheader;

	fileheader.version_major = 2;
	fileheader.version_minor = 4;
	fileheader.magic = 0xa1b23c4d;
	fileheader.thiszone = 0;
	fileheader.linktype = 1;
	fileheader.sigfigs = 0;
	fileheader.snaplen = 65535;

	dumpFile.write(reinterpret_cast<char*>(&fileheader), sizeof(fileheader));

	for (auto& pcapPackage : pcapbuffer) {
		dumpFile.write(reinterpret_cast<char*>(&pcapPackage.header), sizeof(pcapPackage.header));
		dumpFile.write(reinterpret_cast<char*>(pcapPackage.pktData.data()), pcapPackage.pktData.size());
	}

	dumpFile.close();
}
*/