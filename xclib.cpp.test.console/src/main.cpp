#include <cstdio>
#include <tchar.h>
#include "cameras_connected.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	xc::cameras_connected c;
	try
	{
		c.capture();
	}
	catch (const std::exception& ex)
	{
		printf("capture failed: %s", ex.what());
	}
	return 0;
}

