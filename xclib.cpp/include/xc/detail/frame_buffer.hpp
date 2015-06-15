#pragma once

#ifndef XC_FRAME_BUFFER_HPP
#define XC_FRAME_BUFFER_HPP

#include <cstring>
#include <stdexcept>

#include "more\log.hpp"
#include "win32\high_resolution_clock.hpp"
#include "xcliball.h"

#include "xc\detail\util.hpp"

namespace xc
{
	// Allocates and holds resources for a frame buffer.
	// Technically, allocation is performed by the XCLIB driver, we are just acquiring a 
	// pointer to the specified buffer in addition to configuring the byte order.
	class frame_buffer
	{
	public:
		// 
		frame_buffer(pxdstate_s* state, int map, pxbuffer_t id) :
			state(state), map_(map), id_(id), image(nullptr)
		{
			std::memset(&this->dat, 0, sizeof(this->dat));
			this->alloc();
		}

		frame_buffer(frame_buffer&& other) :
			state(other.state), map_(other.map_), id_(other.id_),
			image(other.image), dat(other.dat)
		{
			std::memset(&other.dat, 0, sizeof(other.dat));
			other.image = nullptr;
		}

		//frame_buffer& operator=(frame_buffer&& other)
		//{
		//	this->release();
		//	this->image = other.image;
		//	this->dat = other.dat;
		//	std::memset(&other.dat, 0, sizeof(other.dat));
		//	other.image = nullptr;

		//	return *this;
		//}

		~frame_buffer()
		{
			try
			{
				this->release();
			}
			catch (const std::exception& ex)
			{
				more::error(ex.what());
			}
		}

		frame_buffer(const frame_buffer&) = delete;

		frame_buffer& operator=(const frame_buffer&) = delete;

		// Gets the buffer id.
		pxbuffer_t id() const
		{
			return this->id_;
		}

		// Gets the number of pixel rows.
		size_t height() const
		{
			return this->dat.ydim;
		}

		// Gets the number of image pixels in a row.
		size_t width() const
		{
			return this->dat.xdim;
		}

		// Gets the total number of pixels in a row.
		size_t stride() const
		{
			return this->dat.yinc;
		}

		// Gets the number of bytes in a pixel.
		size_t step() const
		{
			return this->dat.xinc;
		}

		// Gets a constant pointer to the first pixel in the buffer.
		const void* data() const
		{
			return this->dat.p;
		}

		// Gets a high resolution timestamp using (presumably) the windows performance counter.
		win32::high_resolution_clock::time_point get_timestamp() const
		{
			uint32 timestamp_buffer[2];
			int result = ::pxe_buffersSysTicks2(
				this->state, this->map_, this->id_,
				timestamp_buffer);
			detail::handle_result(result);

			typedef win32::high_resolution_clock::time_point time_point;
			typedef win32::high_resolution_clock::duration duration;
			typedef win32::high_resolution_clock::rep rep;
			typedef win32::high_resolution_clock::period period;
			return time_point(duration(
				*reinterpret_cast<uint64_t*>(timestamp_buffer)
				* static_cast<rep>(period::den)
				/ win32::high_resolution_clock::frequency()));
		}

		void alloc()
		{
			this->release();
			this->image = ::pxe_definePximage(
				this->state, this->map_, this->id_, 0, 0, -1, -1, "GREY");
			if (this->image == nullptr)
				throw std::runtime_error("pxe_definePximage");

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

		bool_t is_alloc() const
		{
			return this->image != nullptr;
		}

		void release()
		{
			if (this->is_alloc() == true)
			{
				this->image->imapr(this->image, &this->dat);
				std::memset(&this->dat, 0, sizeof(this->dat));
				::pxe_definePximageFree(this->state, this->image);
				this->image = nullptr;
			}
		}
	private:
		pxdstate_s* const state;
		const int map_;
		const pxbuffer_t id_;
		pximage_s* image;
		pximap dat;
	};
}

#endif//XC_FRAME_BUFFER_HPP