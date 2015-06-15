#pragma once

#include <stdexcept>
#include <Windows.h>

#include "more\includes.hpp"
#include "more\log.hpp"
#include "xcliball.h"

namespace xc
{
	// 
	class capture_event
	{
	public:
		// 
		capture_event(pxdstate_s* state, int map) :
			state(state),
			map(map),
			event_handle(nullptr)
		{
			this->alloc();
		}

		capture_event(capture_event&& other) :
			state(other.state), map(other.map), event_handle(other.event_handle)
		{
			other.event_handle = nullptr;
		}

		capture_event(const capture_event&) = delete;

		capture_event& operator=(const capture_event&) = delete;

		~capture_event()
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
			this->event_handle = ::pxe_eventCapturedFieldCreate(this->state, this->map);
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
				::pxe_eventCapturedFieldClose(this->state, this->map, this->event_handle);
				this->event_handle = nullptr;
			}
		}
	private:
		pxdstate_s* const state;
		const int map;
		HANDLE event_handle;
	};
}