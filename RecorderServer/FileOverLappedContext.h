#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <vector>
class FileOverLappedContext : public OVERLAPPED
{
public:
	FileOverLappedContext();
	~FileOverLappedContext();

	HANDLE FileHandle;
	HANDLE IOPort;
	int Key;
	std::vector<char> buffer;
};

