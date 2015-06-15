#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <Windows.h>

#include "more\includes.hpp"
#include "more\log.hpp"
//#include "rx.hpp"
//#include "win32\event_observable.hpp"
//#include "win32\event.hpp"
#include "xcliball.h"

#include "xc\detail\frame_grabber_options.hpp"
#include "xc\detail\frame_grabber_unit.hpp"
#include "xc\detail\high_resolution_system_time_point.hpp"
#include "xc\detail\util.hpp"
#include "xc\detail\xclib_state.hpp"

namespace xc
{
	// Allocates and manages resources for a set of xclib frame grabber units.
	class frame_grabber
	{
	public:
		// Initializes a frame grabber on the specified ports.
		frame_grabber(
			const std::vector<int>& ports,
			const std::string& format_file_path,
			const frame_grabber_options& options = frame_grabber_options()) :
			state(ports, format_file_path, options),
			clock(detail::high_resolution_system_time_point::now())
		{
			for (const auto& port_info : this->state.ports())
				this->units.emplace_back(
					this->state.handle(),
					port_info,
					this->state.options().buffer_count(),
					this->clock);
		}

		// 
		frame_grabber(frame_grabber&& other) : 
			state(std::move(other.state)),
			units(std::move(other.units)),
			clock(other.clock)
		{
			
		}

		frame_grabber(const frame_grabber&) = delete;

		frame_grabber& operator=(const frame_grabber&) = delete;

		// Checks if the frame grabber is capturing images on all connected units.
		bool_t is_capturing()
		{
			auto result = pxe_goneLive(this->state.handle(), this->state.unitmap(), 0);
			return result == 0 ? false : true;
		}

		// Begins capture on all connected units.
		void start_capture()
		{
			if (this->is_capturing() == false)
			{
				int result = pxe_goLiveSeq(
					this->state.handle(),
					this->state.unitmap(),
					1, static_cast<pxbuffer_t>(this->state.options().buffer_count()),
					1, 0, 1);
				detail::handle_result(result);
			}
		}

		// Ends capture on all connected units.
		void stop_capture()
		{
			if (this->is_capturing() == true)
			{
				int result = pxe_goUnLive(this->state.handle(), this->state.unitmap());
				detail::handle_result(result);
			}
		}

		// Enables the strobe signal.
		void enable_strobe()
		{
			#include "xc\detail\video_trigger_enabled.fmt"
			pxe_videoFormatAsIncludedInit(this->state.handle(), 0);
			int result = pxe_videoFormatAsIncluded(this->state.handle(), 0);
			detail::handle_result(result);
		}

		// Disables the strobe signal.  Cannot perform capture while the strobe is disabled.
		void disable_strobe()
		{
			#include "xc\detail\video_trigger_disabled.fmt"
			pxe_videoFormatAsIncludedInit(this->state.handle(), 0);
			int result = pxe_videoFormatAsIncluded(this->state.handle(), 0);
			detail::handle_result(result);
		}

		// Gets the frame grabber unit on the specified port.
		const frame_grabber_unit& get_unit(int port) const
		{
			for (const auto& unit : units)
				if (unit.port() == port)
					return unit;
			throw std::out_of_range("port");
		}

		//// Removing this functionality for now until i refactor the observables to only return 
		//// the frame grabber index.  Then I can just add a select statement here.
		//rxcpp::observable<image_view> when_image_captured() const
		//{
		//	static std::vector<capture_event> events = [this]()
		//	{
		//		std::vector<capture_event> events;
		//		for (const auto& unit : units)
		//			events.push_back(unit.create_capture_event());
		//		return events;

		//	}();
		//	static win32::manual_reset_event e;

		//	std::vector<HANDLE> wait_handles;
		//	return win32::from_any_event_signalled(wait_handles).map([this](int32_t index) 
		//	{
		//		return this->units[index].get_last_captured_image();
		//	});
		//}

		//// 
		//rxcpp::observable<fault_info> when_fault() const
		//{
		//	return this->fault_observable_ptr->get();
		//}
	private:
		detail::xclib_state state;
		std::vector<frame_grabber_unit> units;
		detail::high_resolution_system_time_point clock;
		//std::unique_ptr<detail::fault_observable> fault_observable_ptr;
		//std::unique_ptr<detail::capture_observable> capture_observable_ptr;
	};
}