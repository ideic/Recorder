#pragma once
#include <memory>
#include "AggregateLogger.h"

class LoggerFactory
{
private:
	LoggerFactory();
	static std::shared_ptr<AggregateLogger> _logger;
public:
	static std::shared_ptr<ILogger> Logger();

	static void InitFileLogger(std::string fileName);

	~LoggerFactory();
};

