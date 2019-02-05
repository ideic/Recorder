#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>


class BlockingQueueTerminated : public std::runtime_error {
public:
	BlockingQueueTerminated() : std::runtime_error("") {}
};


template<typename VALUE_TYPE>
class BlockingQueue {
	const size_t maxSize;
	std::queue<VALUE_TYPE> items;
	mutable std::condition_variable cvNewItem;
	mutable std::condition_variable cvItemRemoved;
	mutable std::mutex mtx;
	bool terminated;

public:
	BlockingQueue(size_t maxSize);
	~BlockingQueue(void);

	void push(VALUE_TYPE&& value);
	VALUE_TYPE getNext();

	void terminate();
};

template<typename VALUE_TYPE>
BlockingQueue<VALUE_TYPE>::BlockingQueue(size_t maxSize) :
	maxSize(maxSize),
	terminated(false)
{
}

template<typename VALUE_TYPE>
BlockingQueue<VALUE_TYPE>::~BlockingQueue(void) {
}

template<typename VALUE_TYPE>
void BlockingQueue<VALUE_TYPE>::push(VALUE_TYPE&& value) {
	std::unique_lock<std::mutex> lock(mtx);

	if (terminated) {
		throw logic_error("BlockingQueue<VALUE_TYPE>::push(VALUE_TYPE&& value) was called in <terminated> state!");
	}

	cvItemRemoved.wait(lock, [this] { return (items.size() < maxSize); });
	items.push(move(value));
	cvNewItem.notify_one();
}

template<typename VALUE_TYPE>
VALUE_TYPE BlockingQueue<VALUE_TYPE>::getNext() {
	std::unique_lock<std::mutex> lock(mtx);

	cvNewItem.wait(lock, [this] { return (!items.empty() || terminated); });

	if (items.empty()) {
		throw BlockingQueueTerminated();
	}

	VALUE_TYPE result = std::move(items.front());
	items.pop();
	cvItemRemoved.notify_one();
	return result;
}

template<typename VALUE_TYPE>
void BlockingQueue<VALUE_TYPE>::terminate() {
	std::unique_lock<std::mutex> lock(mtx);
	terminated = true;
	cvNewItem.notify_all();
}
