#pragma once
#include <WinSock2.h>


struct Context;

class AsyncHandler
{
public:
	AsyncHandler() = default;
	virtual ~AsyncHandler() = default;

	virtual HANDLE getHandle() const = 0;
	virtual void onCompletion(unsigned long transferred, struct Context* context) = 0;
};

