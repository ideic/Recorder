#pragma once
#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <string>
class FileOverLappedContext : public OVERLAPPED
{
public:
	FileOverLappedContext();
	~FileOverLappedContext();

	HANDLE FileHandle;
	HANDLE IOPort;
	uint64_t Key;
	std::vector<char> buffer;
	std::string FileInfoKey;
};

