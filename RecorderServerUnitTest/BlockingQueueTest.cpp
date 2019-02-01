#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\RecorderServer\BlockingQueue.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RecorderServerUnitTest
{		
	class  MyTest
	{
	public:
		MyTest() {};
		~MyTest() {
			int  i = 10 / 5;
		};

	private:

	};

	TEST_CLASS(BlockingQueueTest)
	{
	public:
		
		TEST_METHOD(PushPop)
		{
			BlockingQueue<int> queue;
			queue.push(5);

			auto result = queue.getNext();

			Assert::AreEqual(result, 5);
		}

		TEST_METHOD(SharedPtrReference)
		{
			BlockingQueue<std::shared_ptr<MyTest>> queue;

			{
				std::shared_ptr<MyTest> test = std::make_shared<MyTest>();
				queue.push(std::move(test));

			}
			{
				auto item = queue.getNext();
			}
		}

		TEST_METHOD(TestBlockingCollection)
		{
			BlockingQueue<std::shared_ptr<MyTest>> queue;
			std::atomic_uint32_t deQueudIdx{ 0 };

			std::vector<std::thread> consumers;
			std::vector<std::thread> producers;
			bool terminate{ false };

			const uint32_t limit = 1000000;

			for (uint8_t i = 0; i < 16; i++)
			{
				consumers.push_back(std::thread([&queue, &deQueudIdx, &terminate]() {
					while (!terminate)
					{
						queue.getNext();
						deQueudIdx++;
					}
				}));
			};

			for (uint8_t i = 0; i < 8; i++)
			{
				producers.push_back(std::thread([&queue, &limit]() {
					for (uint32_t i = 0; i < limit; i++)
					{
						std::shared_ptr<MyTest> tmp = std::make_shared<MyTest>();
						queue.push(std::move(tmp));
					}
				}));
			};

			for (auto& i : producers)
			{
				i.join();
			}

			while (deQueudIdx < limit*8) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			};

		//	Assert::AreEqual(std::to_string(deQueudIdx.load()), std::to_string(limit));

			terminate = true;

			for (uint32_t i = 0; i < 8; i++)
			{
				std::shared_ptr<MyTest> tmp = std::make_shared<MyTest>();
				queue.push(std::move(tmp));
			}

			for (auto& i: consumers)
			{
				i.join();
			}
		}
	};
}