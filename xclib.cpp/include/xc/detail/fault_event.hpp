#pragma once

#include <stdexcept>
#include <Windows.h>

#include "xcliball.h"

#include "more\includes.hpp"
#include "more\log.hpp"

namespace xc
{
	// 
	class fault_event
	{
	public:
		fault_event(pxdstate_s* state, int map, int rsvd = 0) :
			state(state),
			map(map),
			rsvd(rsvd),
			event_handle(nullptr)
		{
			this->alloc();
		}

		fault_event(fault_event&& other) :
			state(other.state), map(other.map),
			rsvd(other.rsvd), event_handle(other.event_handle)
		{
			other.event_handle = nullptr;
		}

		fault_event(const fault_event&) = delete;

		fault_event& operator=(const fault_event&) = delete;

		~fault_event()
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

		HANDLE handle() const
		{
			return this->event_handle;
		}

		void alloc()
		{
			this->release();
			this->event_handle = ::pxe_eventFaultCreate(this->state, this->map, this->rsvd);
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
				::pxe_eventFaultClose(
					this->state, this->map,
					this->rsvd, this->event_handle);
				this->event_handle = nullptr;
			}
		}
	private:
		pxdstate_s* const state;
		const int map;
		const int rsvd;
		HANDLE event_handle;
	};
}