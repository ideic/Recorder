#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "cmfxfilehandler.h"
#include "cmfxfilestore.h"
#include "configuration.h"
#include "completionporthandler.h"
#include "stream.h"

using namespace std;


int main(int argc, char *argv[]) {

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	int result = EXIT_SUCCESS;

	try {
		Configuration::getInstance().parseParameters(argc, argv);

		CompletionPortHandler	completionPortHandler(Configuration::getInstance().getIoCompletionThreadCount());
		CmfxFileStore			cmfxFileStore(Configuration::getInstance().getCmfxFileNames());
		shared_ptr<CmfxFile>	cmfxFile = cmfxFileStore.getCmfxFile(Configuration::getInstance().getCmfxFileNames().front());

		AsyncUdpSocketFactory asyncUdpSocketFactory(
			completionPortHandler,
			Configuration::getInstance().getRemoteHost(),
			Configuration::getInstance().getRemotePort());

		CmfxFileHandler cmfxFileHandler(cmfxFile, asyncUdpSocketFactory);
		int i = 0;
		while (i++ < 10) {

			time_t utcTime = time(nullptr);
			cout << asctime(localtime(&utcTime));

			auto startTime = chrono::steady_clock::now();

			unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> nextPackets;
			while ((nextPackets = cmfxFileHandler.getNextPackets()) != nullptr) {
				shared_ptr<AsyncUdpSocket> asyncUdpSocket = nextPackets->first;
				shared_ptr<UdpPacketDataListWithTimeStamp> udpPacketDataList = nextPackets->second;

				this_thread::sleep_until(startTime + udpPacketDataList->timeStamp);

				for (size_t i = 0; i < udpPacketDataList->udpPacketList.size(); ++i) {
					asyncUdpSocket->sendUdpPacket(udpPacketDataList->udpPacketList[i]);
				}
			}

			cmfxFileHandler.resetPosition();

			this_thread::sleep_for(chrono::seconds(1));
		}
	}
	catch (const exception& e) {
		cerr << e.what() << endl;
		result = EXIT_FAILURE;
	}

	WSACleanup();	
	return result;
}