#include "configuration.h"
#include <iostream>
#include "asyncudpsocket.h"

using namespace std;


Configuration& Configuration::getInstance() {
	static Configuration instance;
	return instance;
}

Configuration::Configuration() :
	remoteHost(),
	remotePort(0),
	cmfxFileNames(),
	ioCompletionThreadCount(1)
{
}

Configuration::~Configuration() {
}

void Configuration::parseParameters(int argc, char *argv[]) {

	if (argc != 4) {
		cout << "Usage: " << argv[0] << " <IP_ADDRESS> <PORT> <CMFX_FILE>" << endl;
		throw runtime_error("Invalid command line parameters");
	}

	remoteHost = argv[1];
	remotePort = atoi(argv[2]);

	for (size_t i = 3; i < argc; ++i) {
		cmfxFileNames.push_back(argv[i]);
	}

	ioCompletionThreadCount = 1;
}
