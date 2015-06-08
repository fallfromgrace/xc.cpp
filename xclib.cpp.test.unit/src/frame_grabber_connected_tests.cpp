#include "CppUnitTest.h"
#include "frame_grabber.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace xclibcpptestunit
{
	TEST_CLASS(frame_grabber_connected_tests)
	{
	public:

		TEST_METHOD(test_constructor)
		{
			std::vector<int> v = { 0 };
			try
			{
				xc::frame_grabber fg(v, "C:\\Users\\Light\\Desktop\\format.fmt");
			}
			catch (const std::exception& ex)
			{
				auto what = ex.what();
				auto newsize = strlen(what) + 1;
				wchar_t* wcstring = new wchar_t[newsize];
				size_t convertedChars = 0;
				mbstowcs_s(&convertedChars, wcstring, newsize, what, _TRUNCATE);
				Assert::Fail(wcstring);
			}
		}

	};
}