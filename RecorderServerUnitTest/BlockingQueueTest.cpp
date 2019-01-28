#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\RecorderServer\BlockingQueue.h"
#include <memory>
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
	};
}