#pragma once
#include <list>
#include <map>
#include <memory>
#include <string>

class CmfxFile;


class CmfxFileStore
{
	std::map<std::string, std::shared_ptr<CmfxFile>> store;

	void addFile(const std::string& fileName);

public:
	CmfxFileStore(const std::list<std::string>& fileNames);
	~CmfxFileStore();

	const std::shared_ptr<CmfxFile> getCmfxFile(const std::string& fileName) const;
	const std::map<std::string, std::shared_ptr<CmfxFile>>& getCmfxFiles() const { return store; }
};

