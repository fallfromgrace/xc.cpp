#pragma once

#include <stdexcept>
#include <Windows.h>

#include "xcliball.h"

namespace xc
{
	namespace detail
	{
		// 
		class gpio_event
		{
		public:
			gpio_event(pxdstate_s* instance, int unitmap, int io, int rsvd = 0) :
				instance(instance),
				unitmap(unitmap),
				io(io),
				rsvd(rsvd),
				event_handle(::pxe_eventGPTriggerCreate(instance, unitmap, io, rsvd))
			{
				if (event_handle == nullptr)
					throw std::runtime_error("pxe_eventGPTriggerCreate");
			}

			~gpio_event()
			{
				::pxe_eventGPTriggerClose(
					this->instance,
					this->unitmap,
					this->io,
					this->rsvd,
					this->event_handle);
			}

			HANDLE handle()
			{
				return this->event_handle;
			}
		private:
			pxdstate_s* instance;
			int unitmap;
			int io;
			int rsvd;
			HANDLE event_handle;
		};
	}
}