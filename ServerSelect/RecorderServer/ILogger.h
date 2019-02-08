#pragma once
#include <string>
#include <exception>

class ILogger {
public:
	virtual void LogWarning(const std::string &message)= 0;
	virtual void LogWarning(std::wstring &message);
	virtual void LogInfo(const std::string &message) = 0;
	virtual void LogError(const std::exception &exception, const std::string &message) = 0;
	ILogger() {};
	virtual ~ILogger() {};

	std::string Now();

	std::string ThreadId();
};


