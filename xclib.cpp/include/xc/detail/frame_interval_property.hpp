#pragma once

#include "xc\detail\util.hpp"

#include "more\includes.hpp"
#include "xcliball.h"

namespace xc
{
	class frame_interval_property
	{
	public:
		frame_interval_property(
			pxdstate* state, int unitmap, 
			int32_t pixel_clock, int32_t trigger_duration) :
			state(state), unitmap(unitmap), 
			pixel_clock(pixel_clock), trigger_duration(trigger_duration)
		{

		}

		// Gets the frame interval in microseconds.
		int32_t get() const
		{
			auto prin = pxe_getPrin(this->state, this->unitmap);
			if (prin == 0)
				detail::handle_result(PXERNOTOPEN);

			return prin * (1 << (10 + 0)) / this->pixel_clock + this->trigger_duration + 2;
		}

		// Sets the frame interval in microseconds
		void set(int32_t value) const
		{
			auto exsync = pxe_getExsync(this->state, this->unitmap);
			if (exsync == 0)
				detail::handle_result(PXERNOTOPEN);

			uint prin;
			static const uint prin_max = (1 << 16) - 1;
			int n = 0;
			
			do
			{
				prin = static_cast<uint>(
					(value - this->trigger_duration - 2) * this->pixel_clock / (1 << (10 + n)));
				n++;
			} while (prin > prin_max);

			auto result = pxe_setExsyncPrin(this->state, this->unitmap, exsync, prin);
			detail::handle_result(result);
		}
	private:
		pxdstate* const state;
		const int unitmap;
		const int32_t pixel_clock;//MHz
		const int32_t trigger_duration;//usec
	};
}