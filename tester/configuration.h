#pragma once
#include <list>
#include <string>


class Configuration
{
	std::string remoteHost;
	unsigned short remotePort;
	std::list<std::string> cmfxFileNames;
	size_t ioCompletionThreadCount;
	size_t sendTimes;
	size_t senderThreadCount;

	Configuration();

public:
	~Configuration();

	void parseParameters(int argc, char *argv[]);

	const std::string& getRemoteHost() const { return remoteHost; }
	unsigned short getRemotePort() const { return remotePort; }
	const std::list<std::string>& getCmfxFileNames() const { return cmfxFileNames; }
	size_t getIoCompletionThreadCount() const { return ioCompletionThreadCount; }
	size_t getSendTimes() const { return sendTimes; }
	size_t getSenderThreadCount() const { return senderThreadCount; }

	static Configuration& getInstance();
};

