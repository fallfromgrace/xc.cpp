#pragma once

#include <stdexcept>
#include <Windows.h>

#include "xcliball.h"

namespace xc
{
	namespace detail
	{
		// 
		class fault_event
		{
		public:
			fault_event(pxdstate_s* instance, int unitmap, int rsvd = 0) :
				instance(instance),
				unitmap(unitmap),
				rsvd(rsvd),
				event_handle(::pxe_eventFaultCreate(instance, unitmap, rsvd))
			{
				if (event_handle == nullptr)
					throw std::runtime_error("pxe_eventFaultCreate");
			}

			~fault_event()
			{
				::pxe_eventFaultClose(
					this->instance,
					this->unitmap,
					this->rsvd,
					this->event_handle);
			}

			HANDLE handle() const
			{
				return this->event_handle;
			}
		private:
			pxdstate_s* instance;
			int unitmap;
			int rsvd;
			HANDLE event_handle;
		};
	}
}