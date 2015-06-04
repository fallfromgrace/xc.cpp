#pragma once

#include <vector>

#include "xcliball.h"

#include "frame_buffer.hpp"

namespace xc
{
	namespace detail
	{
		// 
		class frame_buffer_sequence
		{
		public:
			frame_buffer_sequence(pxdstate_s* state, int unitmap, int buffers)
			{
				for (pxbuffer_t buffer = 1; buffer <= buffers; buffer++)
					frame_buffers.emplace_back(state, unitmap, buffer);
			}

			frame_buffer_info get(pxbuffer_t buffer) const
			{
				return this->frame_buffers[buffer - 1].info();
			}

		private:
			std::vector<frame_buffer> frame_buffers;
		};
	}
}