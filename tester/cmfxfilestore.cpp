#include "cmfxfilestore.h"
#include <iostream>
#include <stdexcept>
#include "cmfxfile.h"

using namespace std;


CmfxFileStore::CmfxFileStore(const list<string>& fileNames) {
	for (const auto& fileName : fileNames) {
		addFile(fileName);
	}
}

CmfxFileStore::~CmfxFileStore() {
}

void CmfxFileStore::addFile(const string& fileName) {
	if (store.find(fileName) == store.end()) {
		store.insert(make_pair(fileName, shared_ptr<CmfxFile>(new CmfxFile(fileName))));
	}
}

const shared_ptr<CmfxFile> CmfxFileStore::getCmfxFile(const string& fileName) const {
	auto it = store.find(fileName);

	if (store.end() == it) {
		throw logic_error("CmfxFileStore::getCmfxFile(" + fileName + ")");
	}

	return it->second;
}
