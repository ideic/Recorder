#include "configuration.h"

using namespace std;


Configuration& Configuration::getInstance() {
	static Configuration instance;
	return instance;
}

Configuration::Configuration() {
}

Configuration::~Configuration() {
}

void Configuration::parseParameters(int argc, char *argv[]) {

	if (argc < 4) {
		throw runtime_error("Invalid command line parameters");
	}

	const string remoteHost = argv[1];
	const unsigned short remotePort = atoi(argv[2]);

	for (size_t i = 3; i < argc; ++i) {
		streamStore.addFile(argv[i]);
	}

	ioCompletionThreadCount = 1;

	struct hostent *hp = gethostbyname(remoteHost.c_str());
	if (hp == 0) {
		throw runtime_error("Can not resolve address: " + remoteHost);
	}

	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(remotePort);
	memset(&remoteAddress.sin_addr, 0, sizeof(remoteAddress.sin_addr));
	memcpy_s(&remoteAddress.sin_addr, sizeof(remoteAddress.sin_addr), hp->h_addr, hp->h_length);
}
