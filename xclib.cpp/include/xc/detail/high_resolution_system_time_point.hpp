#pragma once

#include <chrono>

#include "win32\high_resolution_clock.hpp"

namespace xc
{
	namespace detail
	{
		// Combines a high resolution clock with a system clock.  It is only as accurate as the 
		// system clock, but has the precision of the high resolution clock.  This means that 
		// durations between two time points will always be accurate and precise (up to the 
		// precision of the high resolution clock.
		// 
		// TODO move into win32.cpp and rename to clock, making it idiomatic with the other clocks.
		class high_resolution_system_time_point
		{
		public:
			typedef win32::high_resolution_clock high_resolution_clock;
			typedef std::chrono::system_clock system_clock;

			typedef high_resolution_clock::time_point hires_time_point;
			typedef system_clock::time_point system_time_point;

			typedef system_time_point::duration duration;

			// 
			static high_resolution_system_time_point now()
			{
				return high_resolution_system_time_point();
			}

			// Gets a system clock time point from a high resolution clock time point.
			system_time_point get_system_time(hires_time_point t1) const
			{
				return this->time + std::chrono::duration_cast<duration>(t1 - t0);
			}
		private:

			// 
			high_resolution_system_time_point() :
				t0(high_resolution_clock::now()),
				time(system_clock::now())
			{

			}

			hires_time_point t0;
			system_time_point time;
		};
	}
}