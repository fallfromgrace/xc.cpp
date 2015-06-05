#pragma once

#include <chrono>

namespace xc
{
	// 
	class image_info
	{
	public:
		image_info(
			int port,
			size_t width,
			size_t height,
			size_t stride,
			size_t step,
			const void* data,
			std::chrono::system_clock::time_point timestamp) :
			port_(port),
			width_(width),
			height_(height),
			stride_(stride),
			step_(step),
			data_(data),
			timestamp_(timestamp)
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

		const std::chrono::system_clock::time_point& timestamp() const
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
		std::chrono::system_clock::time_point timestamp_;
	};
}