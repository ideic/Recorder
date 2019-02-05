#pragma once
#include <list>
#include <string>
#include <WinSock2.h>


class Configuration
{
	std::string remoteHost;
	unsigned short remotePort;
	std::list<std::string> cmfxFileNames;
	size_t ioCompletionThreadCount;
	size_t sendInstanceCount;
	size_t senderThreadCount;

	Configuration();

public:
	~Configuration();

	void parseParameters(int argc, char *argv[]);

	const std::string& getRemoteHost() const { return remoteHost; }
	unsigned short getRemotePort() const { return remotePort; }
	const std::list<std::string>& getCmfxFileNames() const { return cmfxFileNames; }
	size_t getIoCompletionThreadCount() const { return ioCompletionThreadCount; }
	size_t getSendInstanceCount() const { return sendInstanceCount; }
	size_t getSenderThreadCount() const { return senderThreadCount; }

	static Configuration& getInstance();
};

