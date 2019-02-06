#pragma once
#include "ILogger.h"
#include <string>
#include <fstream>

class FileLogger : public ILogger {
private:
	std::ofstream _file;

public:
	FileLogger(std::string filePath);
	~FileLogger() override;

	virtual void LogWarning(const std::string &message) override;
	virtual void LogInfo(const std::string &message) override;
	virtual void LogError(const std::exception &exception, const std::string &message) override;
};
