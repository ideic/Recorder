#include "completionporthandler.h"
#include <stdexcept>
#include <string>
#include "asynchandler.h"


using namespace std;


CompletionPortHandler::CompletionPortHandler(size_t threadCount) :
	terminated(false),
	workerThreads(threadCount)
{
	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {
		throw runtime_error("CreateIoCompletionPort() failed with error: " + to_string(GetLastError()));
	}

	for (size_t i = 0; i < workerThreads.size(); ++i) {
		workerThreads[i] = std::thread(&CompletionPortHandler::workerFunc, this);
	}
}

CompletionPortHandler::~CompletionPortHandler() {
	terminated = true;

	for (size_t i = 0; i < workerThreads.size(); ++i) {
		workerThreads[i].join();
	}
}

void CompletionPortHandler::attachHandler(const AsyncHandler& asyncHandler) {
	ULONG_PTR completionKey = (ULONG_PTR)&asyncHandler;

	if (NULL == CreateIoCompletionPort(asyncHandler.getHandle(), completionPort, completionKey, 0)) {
		throw runtime_error("CreateIoCompletionPort() failed with error: " + to_string(GetLastError()));
	}
}

void CompletionPortHandler::terminate() {
	terminated = true;
}

void CompletionPortHandler::workerFunc() {
	static const DWORD timeoutMs = 500;
	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = 0;
	PerIoData* perIoData = NULL;

	while (!terminated) {
		if (TRUE == GetQueuedCompletionStatus(completionPort, &bytesTransferred, &completionKey, (LPOVERLAPPED*)&perIoData, timeoutMs)) {
			AsyncHandler* asyncHandler = (AsyncHandler*)completionKey;
			if (NULL == asyncHandler) {
				throw logic_error("CompletionPortHandler::workerFunc(): NULL == asyncPort");
			}

			asyncHandler->onCompletion(bytesTransferred, perIoData);
		}
		else {
			const DWORD lastError = GetLastError();
			if (lastError != WAIT_TIMEOUT) {
				throw runtime_error("GetQueuedCompletionStatus() failed with error: " + to_string(lastError));
			}
		}
	}
}