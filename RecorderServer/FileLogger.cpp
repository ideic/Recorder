#include "stdafx.h"
#include "FileLogger.h"
#include <thread>

FileLogger::FileLogger(std::string logFilePath)
{
	_file.open(logFilePath);

	if (_file.fail()) {
		auto error = strerror(errno);
		throw std::logic_error("Cannot open logFile FileName: " + logFilePath + "Error:" + error);
	}
}


FileLogger::~FileLogger()
{
	if (_file.is_open()) {
		_file.close();
	}
}

void FileLogger::LogWarning(const std::string &message)
{
	_file << "[WARNING]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << message << std::endl;
}

void FileLogger::LogInfo(const std::string &message)
{
	_file << "[INFO]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << message << std::endl;
}

void FileLogger::LogError(const std::exception &exception, const std::string &message)
{
	_file << "[ERROR]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << "Message:" << message << "Exception:" << exception.what() <<std::endl;
}
