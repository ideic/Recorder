#pragma once
#include "ILogger.h"
#include <vector>
#include <memory>
class AggregateLogger: public ILogger
{
private:
	std::vector<std::shared_ptr<ILogger>> _loggers;
public:
	AggregateLogger(std::initializer_list<std::shared_ptr<ILogger>> loggers);
	~AggregateLogger() override;

	virtual void LogWarning(const std::string &message) override;
	virtual void LogInfo(const std::string &message) override;
	virtual void LogError(const std::exception &exception, const std::string &message) override;

	void AddLogger(std::shared_ptr<ILogger> logger);
};

