#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include "asynchandler.h"
#include "udp.h"


class CompletionPortHandler;
struct PerIoData;

class AsyncUdpSocket : public AsyncHandler {
	sockaddr remoteAddress;
	SOCKET socket;

	std::mutex mtx;
	std::map<void*, std::pair<std::shared_ptr<PerIoData>, UdpPacketDataPtr>> perIoDatas;

	void send(PerIoData* perIoData);

public:
	AsyncUdpSocket(CompletionPortHandler& completionPortHandler, const sockaddr* remoteAddress);
	virtual ~AsyncUdpSocket();

	void sendUdpPacket(const UdpPacketDataPtr& udpPacketData);

	virtual HANDLE getHandle() const;
	virtual void onCompletion(unsigned long transferred, PerIoData* perIoData);
};


class AsyncUdpSocketFactory {
	CompletionPortHandler& completionPortHandler;
	std::vector<std::shared_ptr<AsyncUdpSocket>> sockets;
	const std::string remoteHost;
	unsigned short port;

public:
	AsyncUdpSocketFactory(CompletionPortHandler& completionPortHandler, const std::string& remoteHost, unsigned short port);

	std::shared_ptr<AsyncUdpSocket> create(size_t id);
};