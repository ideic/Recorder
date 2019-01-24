#pragma once
#include <map>
#include <memory>
#include <mutex>
#include "asynchandler.h"
#include "udp.h"


class CompletionPortHandler;
struct PerIoData;

class AsyncUdpSocket : public AsyncHandler {
	const sockaddr* remoteAddress;
	SOCKET socket;

	std::mutex mtx;
	std::map<void*, std::shared_ptr<PerIoData>> perIoDatas;

	void send(PerIoData* perIoData);

public:
	AsyncUdpSocket(CompletionPortHandler& completionPortHandler, const sockaddr* remoteAddress);
	virtual ~AsyncUdpSocket();

	void sendUdpPacket(const UdpPacketDataPtr& udpPacketData);

	virtual HANDLE getHandle() const;
	virtual void onCompletion(unsigned long transferred, PerIoData* perIoData);
};

