#include <cstdio>
#include <tchar.h>
#include "frame_grabber.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<int> v = { 1 };
	try
	{
		xc::frame_grabber fg(v, "C:\\Users\\Light\\Desktop\\format.fmt");
		fg.is_capturing();
	}
	catch (const std::exception& ex)
	{
		printf(ex.what());
	}
	return 0;
}

