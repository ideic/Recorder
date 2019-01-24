#include "streamstore.h"
#include <iostream>
#include <stdexcept>

using namespace std;


StreamStore::StreamStore() {
}

StreamStore::~StreamStore() {
}

void StreamStore::addFile(const string& fileName) {
	if (store.find(fileName) == store.end()) {
		store.insert(make_pair(fileName, shared_ptr<Stream>(new Stream(fileName))));
	}
}

const shared_ptr<Stream> StreamStore::getStream(const string& fileName) const {
	auto it = store.find(fileName);

	if (store.end() == it) {
		throw logic_error("StreamStore::getStream(" + fileName + ")");
	}

	return it->second;
}

const std::map<std::string, std::shared_ptr<Stream>> StreamStore::getStreams() const {
	return store;
}
