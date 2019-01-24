#pragma once
#include <list>
#include <string>
#include <WinSock2.h>
#include "streamstore.h"


class Configuration
{
	size_t ioCompletionThreadCount;
	sockaddr_in remoteAddress;
	StreamStore streamStore;

	Configuration();

public:
	~Configuration();

	void parseParameters(int argc, char *argv[]);

	const StreamStore& getStreamStore() const { return streamStore; }
	size_t getIoCompletionThreadCount() const { return ioCompletionThreadCount; }
	const sockaddr* getRemoteAddress() const { return reinterpret_cast<const sockaddr*>(&remoteAddress); }

	static Configuration& getInstance();
};

