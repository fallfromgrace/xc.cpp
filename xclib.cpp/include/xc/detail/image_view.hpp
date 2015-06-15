#pragma once

#include <chrono>

#include "xc\detail\frame_buffer.hpp"
#include "xc\detail\high_resolution_system_time_point.hpp"

namespace xc
{
	// 
	class image_view
	{
	public:
		typedef std::chrono::system_clock::time_point time_point;

		image_view(
			int port,
			const frame_buffer& frame_buffer,
			const detail::high_resolution_system_time_point& clock) :
			port_(port),
			width_(frame_buffer.width()),
			height_(frame_buffer.height()),
			stride_(frame_buffer.stride()),
			step_(frame_buffer.step()),
			data_(frame_buffer.data()),
			timestamp_(clock.get_system_time(frame_buffer.get_timestamp()))
		{

		}

		int port() const
		{
			return this->port_;
		}

		size_t width() const
		{
			return this->width_;
		}

		size_t height() const
		{
			return this->height_;
		}

		size_t stride() const
		{
			return this->stride_;
		}

		size_t step() const
		{
			return this->step_;
		}

		const void* data() const
		{
			return this->data_;
		}

		const time_point& timestamp() const
		{
			return this->timestamp_;
		}
	private:
		size_t width_;
		size_t height_;
		size_t stride_;
		size_t step_;
		const void* data_;
		int port_;
		time_point timestamp_;
	};
}