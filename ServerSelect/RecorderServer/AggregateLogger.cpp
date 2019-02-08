#include "stdafx.h"
#include "AggregateLogger.h"


AggregateLogger::AggregateLogger(std::initializer_list<std::shared_ptr<ILogger>> loggers)
{
	for (auto & logger : loggers) {
		_loggers.push_back(logger);
	}
}


AggregateLogger::~AggregateLogger()
{
}

void AggregateLogger::LogWarning(const std::string &message)
{
	for (auto & logger : _loggers) {
		logger->LogWarning(message);
	}
}

void AggregateLogger::LogInfo(const std::string &message)
{
	for (auto & logger : _loggers) {
		logger->LogInfo(message);
	}
}

void AggregateLogger::LogError(const std::exception &exception, const  std::string &message)
{
	for (auto & logger : _loggers) {
		logger->LogError(exception, message);
	}
}

void AggregateLogger::AddLogger(const std::shared_ptr<ILogger> &logger)
{
	_loggers.push_back(logger);
}
