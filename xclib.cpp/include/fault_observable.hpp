#pragma once

#include <thread>
#include <Windows.h>

#include "rx.hpp"
#include "win32\win32_event.hpp"
#include "win32\win32_exception.hpp"
#include "xcliball.h"

#include "fault_event.hpp"
#include "frame_grabber_unit.hpp"

namespace xc
{
	// 
	class fault_info
	{
	public:

		fault_info(int port, const std::string& message) :
			port_(port), message(message)
		{

		}

		int port() const
		{
			return this->port_;
		}

		const std::string& what() const
		{
			return this->message;
		}

	private:

		int port_;
		std::string message;
	};

	namespace detail
	{
		// 
		class fault_observable
		{
		public:
			// 
			fault_observable(
				pxdstate* state,
				const std::vector<frame_grabber_unit_info>& units) :
				state(state),
				units(units)
			{
				for (const auto& unit_info : this->units)
					this->wait_handles.emplace_back(unit_info.fault_event_handle());
				this->wait_handles.emplace_back(this->close_event.handle());

				this->thread = std::thread(this, &fault_observable::observer);
			}

			~fault_observable()
			{
				if (this->thread.joinable() == true)
				{
					this->close_event.set();
					this->thread.join();
				}
			}

			rxcpp::observable<fault_info> get() const
			{
				return this->fault_subject.get_observable();
			}

		private:

			bool_t on_wait_success(DWORD wait_handle_index)
			{
				if (wait_handle_index == units.size())
				{
					this->observer.on_completed();
					return false;
				}
				else
				{
					auto unit = this->units[wait_handle_index];

					std::array<char, 1024> buffer;
					int fault_result = ::pxe_mesgFaultText(this->state, unit.map(), buffer.data(), buffer.size());
					if (fault_result == 1)
						this->observer.on_next(fault_info(unit.port(), buffer.data()));
					else if (fault_result == 0)
						log_warn("Fault event triggered but no fault recorded by XCLIB.");
					else if (fault_result > 0)
						log_warn(::pxd_mesgErrorCode(fault_result));
					else
						log_error(::pxd_mesgErrorCode(fault_result));
					return true;
				}
			}

			bool_t on_wait_abandoned(DWORD wait_handle_index)
			{
				if (wait_handle_index == units.size())
				{
					this->observer.on_next(fault_info(0, "close wait handle abandoned unexpectedly"));
				}
				else
				{
					auto unit = this->units[wait_handle_index];
					this->observer.on_next(fault_info(unit.port(), "fault wait handle abandoned unexpectedly"));
				}
				this->observer.on_completed();
				return false;
			}

			bool_t on_wait_timeout()
			{
				this->observer.on_next(fault_info(0, "fault wait handle timed out unexpectedly"));
				this->observer.on_completed();
				return false;
			}

			bool_t on_wait_failed()
			{
				this->observer.on_next(fault_info(0, win32::get_last_win32_error_message()));
				this->observer.on_completed();
				return false;
			}

			void observer()
			{
				bool_t observing = true;
				while (observing == true)
				{
					DWORD wait_result = ::WaitForMultipleObjects(
						this->wait_handles.size(),
						this->wait_handles.data(),
						false, INFINITE);

					if (wait_result == WAIT_FAILED)
						observing = on_wait_failed();
					else if (wait_result == WAIT_TIMEOUT)
						observing = on_wait_timeout();
					else if (wait_result & WAIT_ABANDONED_0 == WAIT_ABANDONED_0)
						observing = on_wait_abandoned(wait_result - WAIT_ABANDONED_0);
					else if (wait_result & WAIT_OBJECT_0 == WAIT_OBJECT_0)
						observing = on_wait_success(wait_result - WAIT_OBJECT_0);
					else
						observing = on_wait_failed();
				}
			}

			pxdstate* state;
			rxcpp::observer<fault_info> observer;
			rxcpp::observable<fault_info> observable;
			rxcpp::subjects::subject<fault_info> fault_subject;
			std::vector<HANDLE> wait_handles;
			std::vector<frame_grabber_unit_info> units;
			win32::manual_reset_event close_event;
			std::thread thread;
		};
	}
}