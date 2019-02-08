#pragma once
#include "ILogger.h"
class ConsoleLogger: public ILogger
{
public:
	ConsoleLogger();
	~ConsoleLogger();

	virtual void LogWarning(const std::string &message) override;
	virtual void LogInfo(const std::string &message) override;
	virtual void LogError(const std::exception &exception, const std::string &message) override;
};

