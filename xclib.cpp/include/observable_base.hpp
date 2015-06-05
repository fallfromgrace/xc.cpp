#pragma once

#include <vector>
#include <Windows.h>

#include "rx.hpp"
#include "win32\win32_event.hpp"

namespace win32
{
	template<typename value>
	class observable_base
	{
	public:
		// 
		observable_base(const std::vector<HANDLE>& wait_handles) :
			wait_handles(wait_handles)
		{
			this->connect();
		}

		observable_base(const observable_base&) = delete;

		observable_base& operator=(const observable_base&) = delete;

		// 
		~observable_base()
		{
			this->disconnect();
		}

		// 
		rxcpp::observable<value> get() const
		{
			return this->subject.get_observable();
		}

		// 
		void connect()
		{
			this->disconnect();
			this->thread = std::thread(this, &observable_base::observe);
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
	protected:

		template<typename observer>
		virtual value get_next(DWORD index) = 0;

	private:
		// 
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
					observer.on_next(this->get_next(wait_handle_index));
				}
				catch (const std::exception& ex)
				{
					log_error(std::string(ex.what()));
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
				log_error("Close wait handle abandoned unexpectedly.");
			else
				log_error("Wait handle %i abandoned unexpectedly.", wait_handle_index);
			observer.on_completed();
			return false;
		}

		template<typename observer>
		bool_t on_wait_timeout(observer&& observer)
		{
			log_error("Wait timed out unexpectedly.");
			observer.on_completed();
			return false;
		}

		template<typename observer>
		bool_t on_wait_failed(observer&& observer)
		{
			log_error(win32::get_last_win32_error_message());
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

		rxcpp::subjects::subject<value> subject;
		std::vector<HANDLE> wait_handles;
		win32::manual_reset_event close_event;
		std::thread thread;
	};
}