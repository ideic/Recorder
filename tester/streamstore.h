#pragma once
#include <map>
#include <memory>
#include <string>
#include "stream.h"


class StreamStore
{
	std::map<std::string, std::shared_ptr<Stream>> store;

public:
	StreamStore();
	~StreamStore();

	void addFile(const std::string& fileName);
	const std::shared_ptr<Stream> getStream(const std::string& fileName) const;
	const std::map<std::string, std::shared_ptr<Stream>> getStreams() const;
};

