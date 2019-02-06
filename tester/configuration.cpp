#include "configuration.h"
#include <iostream>

using namespace std;


Configuration& Configuration::getInstance() {
	static Configuration instance;
	return instance;
}

Configuration::Configuration() :
	remoteHost(),
	remotePort(0),
	cmfxFileNames(),
	ioCompletionThreadCount(1),
	sendTimes(1),
	senderThreadCount(2)
{
}

Configuration::~Configuration() {
}

void Configuration::parseParameters(int argc, char *argv[]) {

	if (argc < 5) {
		cout << "usage: [IP ADDRESS] [PORT] [SEND TIMES] [CMFX FILE]" << endl;
		throw runtime_error("Invalid command line parameters");
	}

	remoteHost = argv[1];
	remotePort = stoi(argv[2]);
	sendTimes = stoi(argv[3]);

	for (size_t i = 4; i < argc; ++i) {
		cmfxFileNames.push_back(argv[i]);
	}
}
