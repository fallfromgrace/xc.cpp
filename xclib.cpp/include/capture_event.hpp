#pragma once

#include <stdexcept>
#include <Windows.h>

#include "xcliball.h"

namespace xc
{
	namespace detail
	{
		// 
		class capture_event
		{
		public:
			// 
			capture_event(pxdstate_s* instance, int unitmap) : 
				instance(instance), 
				unitmap(unitmap), 
				event_handle(::pxe_eventCapturedFieldCreate(instance, unitmap))
			{
				if (event_handle == nullptr)
					throw std::runtime_error("pxe_eventCapturedFieldCreate");
			}

			~capture_event()
			{
				::pxe_eventCapturedFieldClose(this->instance, this->unitmap, this->event_handle);
			}

			HANDLE handle() const
			{
				return this->event_handle;
			}
		private:
			pxdstate_s* instance;
			int unitmap;
			HANDLE event_handle;
		};
	}
}