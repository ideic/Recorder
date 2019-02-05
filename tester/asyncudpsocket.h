#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include "asynchandler.h"
#include "udp.h"


class CompletionPortHandler;
struct Context;

class AsyncUdpSocket : public AsyncHandler {
	sockaddr remoteAddress;
	SOCKET socket;

	std::mutex mtx;
	std::map<void*, std::pair<std::shared_ptr<struct Context>, UdpPacketDataPtr>> contexts;

	void send(struct Context* context);

public:
	AsyncUdpSocket(CompletionPortHandler& completionPortHandler, const sockaddr* remoteAddress);
	virtual ~AsyncUdpSocket();

	void sendUdpPacket(const UdpPacketDataPtr& udpPacketData);

	virtual HANDLE getHandle() const override;
	virtual void onCompletion(unsigned long transferred, struct Context* context) override;
};


class AsyncUdpSocketFactory {
	CompletionPortHandler& completionPortHandler;
	const std::string remoteHost;
	const unsigned short port;
	unsigned short portIncrement;

public:
	AsyncUdpSocketFactory(CompletionPortHandler& completionPortHandler, const std::string& remoteHost, unsigned short port);

	std::shared_ptr<AsyncUdpSocket> create();
};