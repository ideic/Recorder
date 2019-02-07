#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <WinSock2.h>


class AsyncHandler;

struct Context : public OVERLAPPED {
	WSABUF wsaBuffer;
};


class CompletionPortHandler {
	HANDLE completionPort;
	std::atomic_bool terminated;
	std::vector<std::thread> workerThreads;

	void workerFunc();

public:
	CompletionPortHandler(size_t threadCount);
	~CompletionPortHandler();

	void attachHandler(const AsyncHandler& asyncHandler);
	void terminate();
};

