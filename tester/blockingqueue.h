#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>



#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>


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
	bool hasNext() const;
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

// hasNext() and getNext() is used only by HttpRequestSender::workerFunction()
template<typename VALUE_TYPE>
bool BlockingQueue<VALUE_TYPE>::hasNext() const {
	std::unique_lock<std::mutex> lock(mtx);
	cvNewItem.wait(lock, [this] { return (!items.empty() || terminated); });
	return !items.empty();
}

// hasNext() and getNext() is used only by HttpRequestSender::workerFunction()
// getNext() must be called after hasNext() returns true!!!!
template<typename VALUE_TYPE>
VALUE_TYPE BlockingQueue<VALUE_TYPE>::getNext() {
	std::unique_lock<std::mutex> lock(mtx);

	if (items.empty()) {
		throw logic_error("BlockingQueue::getNext() is called when the queue is empty!");
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


/*
template<typename VALUE_TYPE>
class BlockingQueue {
	const size_t maxSize;
	std::queue<VALUE_TYPE> items;
	mutable std::condition_variable cv;
	mutable std::mutex mtx;
	bool terminated;

public:
	BlockingQueue(size_t maxSize);
	~BlockingQueue(void);

	void push(VALUE_TYPE&& value);
	bool hasNext() const;
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

	items.push(move(value));
	cv.notify_one();
}

// hasNext() and getNext() is used only by HttpRequestSender::workerFunction()
template<typename VALUE_TYPE>
bool BlockingQueue<VALUE_TYPE>::hasNext() const {
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [this]{return (!items.empty() || terminated);});
	return !items.empty();
}

// hasNext() and getNext() is used only by HttpRequestSender::workerFunction()
// getNext() must be called after hasNext() returns true!!!!
template<typename VALUE_TYPE>
VALUE_TYPE BlockingQueue<VALUE_TYPE>::getNext() {
	std::unique_lock<std::mutex> lock(mtx);

	if (items.empty()) {
		throw logic_error("BlockingQueue::getNext() is called when the queue is empty!");
	}

	VALUE_TYPE result = std::move(items.front());
	items.pop();
	return result;
}

template<typename VALUE_TYPE>
void BlockingQueue<VALUE_TYPE>::terminate() {
	std::unique_lock<std::mutex> lock(mtx);
	terminated = true;
	cv.notify_all();
}
*/