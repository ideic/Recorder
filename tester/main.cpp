#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include "asyncudpsocket.h"
#include "cmfxfile.h"
#include "cmfxfilehandler.h"
#include "configuration.h"
#include "completionporthandler.h"
#include "packetsender.h"
#include "stream.h"

using namespace std;


ostream& operator<<(ostream& os, const chrono::time_point<chrono::system_clock>& time) {
	time_t t = chrono::system_clock::to_time_t(time);
	os << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
	return os;
}

int main(int argc, char *argv[]) {

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	int result = EXIT_SUCCESS;

	try {
		Configuration::getInstance().parseParameters(argc, argv);

		CompletionPortHandler completionPortHandler(Configuration::getInstance().getIoCompletionThreadCount());
		AsyncUdpSocketFactory asyncUdpSocketFactory(completionPortHandler, Configuration::getInstance().getRemoteHost(), Configuration::getInstance().getRemotePort());

		CmfxFileHandler cmfxFileHandler(asyncUdpSocketFactory, Configuration::getInstance().getSendTimes());
		for (auto fileName : Configuration::getInstance().getCmfxFileNames()) {
			cmfxFileHandler.addCmfxFile(fileName);
		}
		cmfxFileHandler.mergeFiles();

		cout << chrono::system_clock::now() << " Starting to send " << cmfxFileHandler.getStreamCount() << " streams at " << Configuration::getInstance().getSendTimes() << " times" << endl;

		BlockingQueueUdpPacket queue(Configuration::getInstance().getSendTimes() * Configuration::getInstance().getCmfxFileNames().size());
		PacketSender packetSender(queue, Configuration::getInstance().getSenderThreadCount(), chrono::steady_clock::now());
		shared_ptr<pair<shared_ptr<AsyncUdpSocket>, shared_ptr<UdpPacketDataListWithTimeStamp>>> packetListWithSocket;

		while ((packetListWithSocket = cmfxFileHandler.getNextPacketListWithSocket()) != nullptr) {
			queue.push(packetListWithSocket);
		}

		queue.terminate();
		packetSender.join();

		cout << chrono::system_clock::now() << " Finnished" << endl;
	}
	catch (const exception& e) {
		cerr << e.what() << endl;
		result = EXIT_FAILURE;
	}

	WSACleanup();	
	return result;
}