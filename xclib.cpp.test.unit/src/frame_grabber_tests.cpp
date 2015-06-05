#include "CppUnitTest.h"
#include "frame_grabber.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace xclibcpptestunit
{
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(TestMethod1)
		{
			std::vector<int> v = { 1 };
			try
			{
				xc::frame_grabber fg("C:\\Users\\Light\\Desktop\\format.fmt", v);
				fg.is_capturing();
			}
			catch (const std::exception& ex)
			{
				printf(ex.what());
			}
		}

	};
}