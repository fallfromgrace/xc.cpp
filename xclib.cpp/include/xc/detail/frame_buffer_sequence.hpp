#pragma once

#include <vector>

#include "xcliball.h"

#include "xc\detail\frame_buffer.hpp"

namespace xc
{
	// This class solely exists to allow getting frame buffer by id.
	class frame_buffer_sequence
	{
	public:
		// 
		frame_buffer_sequence(pxdstate_s* state, int unitmap, size_t buffers)
		{
			for (pxbuffer_t id = 1; id <= buffers; id++)
				frame_buffers.emplace_back(state, unitmap, id);
		}

		// 
		frame_buffer_sequence(frame_buffer_sequence&& other) :
			frame_buffers(std::move(other.frame_buffers))
		{

		}

		//// 
		//frame_buffer_sequence& operator=(frame_buffer_sequence&& other)
		//{
		//	this->frame_buffers = std::move(other.frame_buffers);

		//	return *this;
		//}

		frame_buffer_sequence(const frame_buffer_sequence&) = delete;

		frame_buffer_sequence& operator=(const frame_buffer_sequence&) = delete;

		// Gets the frame buffer with the specified id.
		frame_buffer& get(pxbuffer_t id)
		{
			return this->frame_buffers[id - 1];
		}

		// Gets the frame buffer with the specified id.
		const frame_buffer& get(pxbuffer_t id) const
		{
			return this->frame_buffers[id - 1];
		}

		// Gets the number of frame buffers in this sequence.
		size_t size() const
		{
			return this->frame_buffers.size();
		}
	private:
		std::vector<frame_buffer> frame_buffers;
	};
}