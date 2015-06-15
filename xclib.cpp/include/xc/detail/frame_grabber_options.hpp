#pragma once

namespace xc
{
	// 
	class frame_grabber_options
	{
	public:
		// 
		frame_grabber_options(
			size_t buffer_count = 10) :
			buffer_count_(buffer_count)
		{

		}

		// 
		size_t buffer_count() const
		{
			return this->buffer_count_;
		}

	private:
		size_t buffer_count_;
	};
}