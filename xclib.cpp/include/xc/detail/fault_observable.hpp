#pragma once

#include <thread>
#include <Windows.h>

#include "more\log.hpp"
#include "rx.hpp"
#include "win32\event.hpp"
#include "win32\error.hpp"
#include "xcliball.h"

#include "fault_info.hpp"
#include "fault_event.hpp"
#include "frame_grabber_unit.hpp"

namespace xc
{
	namespace detail
	{
		// 
		class fault_observable
		{
		public:
			// 
			fault_observable(
				const std::vector<std::unique_ptr<frame_grabber_unit>>& units)
			{
				for (const auto& unit : units)
				{
					this->units.push_back(unit.get());
					this->wait_handles.emplace_back(unit->fault_event().handle());
				}
				this->wait_handles.emplace_back(this->close_event.handle());

				this->connect();
			}

			fault_observable(const fault_observable&) = delete;

			fault_observable& operator=(const fault_observable&) = delete;

			~fault_observable()
			{
				try
				{
					this->disconnect();
				}
				catch (const std::exception& ex)
				{
					more::error(ex.what());
				}
			}

			// 
			void connect()
			{
				this->disconnect();
				this->thread = std::thread(&fault_observable::observe, this);
			}

			// 
			void disconnect()
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

			template<typename observer>
			bool_t on_wait_success(
				observer&& observer, 
				DWORD wait_handle_index)
			{
				if (wait_handle_index == units.size())
				{
					observer.on_completed();
					return false;
				}
				else
				{
					try
					{
						this->units[wait_handle_index]->check_fault();
						logging::warn("Fault event triggered but no fault recorded by XCLIB.");
					}
					catch (const std::exception& ex)
					{
						observer.on_next(fault_info(
							this->units[wait_handle_index]->port(), 
							std::string(ex.what())));
					}
					return true;
				}
			}

			template<typename observer>
			bool_t on_wait_abandoned(
				observer&& observer,
				DWORD wait_handle_index)
			{
				if (wait_handle_index == units.size())
					logging::error("Close wait handle abandoned unexpectedly.");
				else
					logging::error("Fault wait handle for unit on port %i abandoned unexpectedly.",
						this->units[wait_handle_index]->port());
				observer.on_completed();
				return false;
			}

			template<typename observer>
			bool_t on_wait_timeout(observer&& observer)
			{
				logging::error("Wait timed out unexpectedly.");
				observer.on_completed();
				return false;
			}

			template<typename observer>
			bool_t on_wait_failed(observer&& observer)
			{
				logging::error(win32::get_last_error().what());
				observer.on_completed();
				return false;
			}

			void observe()
			{
				bool_t observing = true;

				auto observer = this->fault_subject.get_subscriber().get_observer();
				while (observing == true)
				{
					DWORD wait_result = ::WaitForMultipleObjects(
						static_cast<DWORD>(this->wait_handles.size()),
						this->wait_handles.data(),
						false, INFINITE);

					if (wait_result == WAIT_FAILED)
						observing = on_wait_failed(observer);
					else if (wait_result == WAIT_TIMEOUT)
						observing = on_wait_timeout(observer);
					else if ((wait_result & WAIT_ABANDONED_0) == WAIT_ABANDONED_0)
						observing = on_wait_abandoned(observer, wait_result - WAIT_ABANDONED_0);
					else if ((wait_result & WAIT_OBJECT_0) == WAIT_OBJECT_0)
						observing = on_wait_success(observer, wait_result - WAIT_OBJECT_0);
					else
						observing = on_wait_failed(observer);
				}
			}

			win32::manual_reset_event close_event;
			std::thread thread;

			rxcpp::subjects::subject<fault_info> fault_subject;

			std::vector<HANDLE> wait_handles;
			std::vector<frame_grabber_unit*> units;
		};
	}
}