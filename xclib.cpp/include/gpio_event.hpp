#pragma once

#include <stdexcept>
#include <Windows.h>

#include "includes.hpp"
#include "logging\logging.hpp"
#include "xcliball.h"

namespace xc
{
	namespace detail
	{
		// 
		class gpio_event
		{
		public:
			gpio_event(pxdstate_s* state, int map, int io, int rsvd = 0) :
				state(state),
				map(map),
				io(io),
				rsvd(rsvd),
				event_handle(nullptr)
			{
				this->alloc();
			}

			gpio_event(gpio_event&& other) :
				state(other.state), map(other.map), io(other.io), 
				rsvd(other.rsvd), event_handle(other.event_handle)
			{
				other.event_handle = nullptr;
			}

			gpio_event(const gpio_event&) = delete;

			gpio_event& operator=(const gpio_event&) = delete;

			~gpio_event()
			{
				try
				{
					this->release();
				}
				catch (const std::exception& ex)
				{
					log_error(ex.what());
				}
			}

			HANDLE handle() const
			{
				return this->event_handle;
			}

			void alloc()
			{
				this->release();
				this->event_handle = ::pxe_eventGPTriggerCreate(
					this->state, this->map, 
					this->io, this->rsvd);
				if (this->event_handle == nullptr)
					throw std::runtime_error("pxe_eventCapturedFieldCreate");
			}

			bool_t is_alloc() const
			{
				return this->event_handle != nullptr;
			}

			void release()
			{
				if (this->is_alloc() == true)
				{
					::pxe_eventGPTriggerClose(
						this->state, this->map,
						this->io, this->rsvd,
						this->event_handle);
					this->event_handle = nullptr;
				}
			}
		private:
			pxdstate_s* const state;
			const int map;
			const int io;
			const int rsvd;
			HANDLE event_handle;
		};
	}
}