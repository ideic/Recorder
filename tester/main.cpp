#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include "asyncudpsocket.h"
#include "configuration.h"
#include "completionporthandler.h"
#include "stream.h"


#include "header.h"
using namespace std;


int main(int argc, char *argv[]) {

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	int result = EXIT_SUCCESS;

	try {
		Configuration::getInstance().parseParameters(argc, argv);

		CompletionPortHandler completionPortHandler(Configuration::getInstance().getIoCompletionThreadCount());
		AsyncUdpSocket udpSocket(completionPortHandler, Configuration::getInstance().getRemoteAddress());

		while (true) {
			for (auto streamStoreItem : Configuration::getInstance().getStreamStore().getStreams()) {
				const string& fileName = streamStoreItem.first;
				const shared_ptr<Stream> stream = streamStoreItem.second;

				time_t utcTime = time(nullptr);
				cout << asctime(localtime(&utcTime));
				cout << "  sending: " << fileName << endl;

				auto startTime = chrono::steady_clock::now();
				for (const auto& udpPacketDataList : stream->getUdpPacketDatas()) {
					/*
					cout << "ts: " << udpPacketDataList.timeStamp.count() << endl;
					for (size_t i = 0; i < udpPacketDataList.udpPacketList.size(); ++i) {
					cout << "    " << setw(2) << setfill(' ') << i << " length: " << udpPacketDataList.udpPacketList[i]->length + 42 << endl;
					}
					*/
					this_thread::sleep_until(startTime + udpPacketDataList.timeStamp);
					for (size_t i = 0; i < udpPacketDataList.udpPacketList.size(); ++i) {
						udpSocket.sendUdpPacket(udpPacketDataList.udpPacketList[i]);
					}
				}

				this_thread::sleep_for(chrono::seconds(1));
			}
		}
	}
	catch (const exception& e) {
		cerr << e.what() << endl;
		result = EXIT_FAILURE;
	}

	WSACleanup();	
	return result;
}