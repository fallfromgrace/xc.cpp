#pragma once

#include "xcliball.h"

namespace xc
{
	// 
	class frame_buffer_info
	{
	public:
		// 
		frame_buffer_info(const pximap& image_data) :
			image_data(image_data)
		{

		}

		size_t height() const
		{
			return this->image_data.ydim;
		}

		size_t width() const
		{
			return this->image_data.xdim;
		}

		size_t stride() const
		{
			return this->image_data.yinc;
		}

		size_t step() const
		{
			return this->image_data.xinc;
		}

		const void* data() const
		{
			return this->image_data.p;
		}
	private:
		const pximap image_data;
	};
}