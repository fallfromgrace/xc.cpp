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
			frame_buffer_sequence(pxdstate_s* state, int unitmap, size_t buffers)
			{
				for (pxbuffer_t id = 1; id <= buffers; id++)
					frame_buffers.emplace_back(state, unitmap, id);
			}

			frame_buffer_sequence(frame_buffer_sequence&& other) : 
				frame_buffers(std::move(other.frame_buffers))
			{

			}

			frame_buffer_sequence(const frame_buffer_sequence&) = delete;

			frame_buffer_sequence& operator=(const frame_buffer_sequence&) = delete;

			const frame_buffer& get(pxbuffer_t buffer) const
			{
				return this->frame_buffers[buffer - 1];
			}

			size_t size() const
			{
				return this->frame_buffers.size();
			}

		private:
			std::vector<frame_buffer> frame_buffers;
		};
	}
}