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
	std::string Key;
	std::vector<char> buffer;
};

