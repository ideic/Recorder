#include "stdafx.h"
#include "LoggerFactory.h"
#include "FileLogger.h"
#include "AggregateLogger.h"
#include "ConsoleLogger.h"


std::shared_ptr<AggregateLogger> LoggerFactory::_logger(new AggregateLogger({ std::make_shared<ConsoleLogger>() }));
LoggerFactory::LoggerFactory()
{
}


std::shared_ptr<ILogger> LoggerFactory::Logger()
{
	return _logger;
}

void LoggerFactory::InitFileLogger(std::string fileName)
{
	_logger->AddLogger(std::make_shared<FileLogger>(fileName));
}

LoggerFactory::~LoggerFactory()
{
}
