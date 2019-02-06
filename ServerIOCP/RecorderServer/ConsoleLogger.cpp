#include "stdafx.h"
#include "ConsoleLogger.h"
#include <iostream>

ConsoleLogger::ConsoleLogger()
{
}


ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::LogWarning(const std::string & message)
{
	std::cout << "[WARNING]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << message << std::endl;
}

void ConsoleLogger::LogInfo(const std::string & message)
{
	std::cout << "[INFO]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << message << std::endl;
}

void ConsoleLogger::LogError(const std::exception & exception, const std::string & message)
{
	std::cerr << "[ERROR]" << ThreadId() << std::string(" > ") << Now() << std::string(" | ") << "Message:" << message << "Exception:" << exception.what() << std::endl;
}
