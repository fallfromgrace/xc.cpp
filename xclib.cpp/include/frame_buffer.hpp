#ifndef XCLIB_XCBUFFER_HPP
#define XCLIB_XCBUFFER_HPP

#include <cstring>
#include <stdexcept>

#include "xcliball.h"

#include "frame_buffer_info.hpp"

namespace xc
{
	namespace detail
	{
		// Acquires and holds resources for a frame buffer.
		class frame_buffer
		{
		public:
			// 
			frame_buffer(pxdstate_s* state, int unitmap, pxbuffer_t buffer) :
				image(nullptr), state(state)
			{
				this->image = ::pxe_definePximage(this->state, unitmap, buffer, 0, 0, -1, -1, "GREY");
				if (this->image != nullptr)
					throw std::runtime_error("pxe_definePximage");

				std::memset(&this->dat, 0, sizeof(this->dat));

				int imapset_result = this->image->imapset(
					this->image,
					PXRXSCAN | PXIWRAP,
					PXDATUCHAR,
					0x01);
				if (imapset_result <= 0)
					throw std::runtime_error("imapset");

				int imap_result = this->image->imap(this->image, &this->dat, 0, 0);
				if (imap_result <= 0)
					throw std::runtime_error("imap");
			}

			frame_buffer(const frame_buffer&) = delete;

			frame_buffer& operator=(const frame_buffer&) = delete;

			~frame_buffer()
			{
				this->image->imapr(this->image, &this->dat);
				::pxe_definePximageFree(this->state, this->image);
			}

			// Gets info about this frame buffer.
			frame_buffer_info info() const
			{
				return frame_buffer_info(dat);
			}
		private:
			pximap dat;
			pximage_s* image;
			pxdstate_s* state;
		};
	}
}

#endif//XCLIB_XCBUFFER_HPP