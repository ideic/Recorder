#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "cmfxfilestore.h"
#include "cmfxhandlerstore.h"
#include "configuration.h"
#include "completionporthandler.h"
#include "packetsender.h"
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
		CmfxHandlerStore		cmfxHandlerStore;

		AsyncUdpSocketFactory asyncUdpSocketFactory(completionPortHandler,
			Configuration::getInstance().getRemoteHost(),
			Configuration::getInstance().getRemotePort());

		size_t streamCount = 0;

		for (size_t i = 0; i < Configuration::getInstance().getSendInstanceCount(); ++i) {
			for (auto fileName : Configuration::getInstance().getCmfxFileNames()) {
				const shared_ptr<CmfxFile> cmfxFile = cmfxFileStore.getCmfxFile(fileName);
				cmfxHandlerStore.add(cmfxFile, asyncUdpSocketFactory);
				streamCount += cmfxFile->getStreamCount();
			}
		}

		cout << "Starting to send " << streamCount << " streams" << endl;

		BlockingQueueUdpPacket queue(100);
		PacketSender packetSender(queue, Configuration::getInstance().getSenderThreadCount(), chrono::steady_clock::now());

		unique_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> packetListWithSocket;
		while ((packetListWithSocket = cmfxHandlerStore.getNextPacketListWithSocket()) != nullptr) {
			queue.push(move(packetListWithSocket));
		}

		queue.terminate();
		packetSender.join();
	}
	catch (const exception& e) {
		cerr << e.what() << endl;
		result = EXIT_FAILURE;
	}

	WSACleanup();	
	return result;
}