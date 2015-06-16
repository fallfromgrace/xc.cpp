#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <Windows.h>

#include "more\includes.hpp"
#include "more\log.hpp"
#include "rx.hpp"
#include "win32\event_observable.hpp"
#include "win32\event.hpp"
#include "xcliball.h"

#include "xc\detail\fault_info.hpp"
#include "xc\detail\frame_grabber_options.hpp"
#include "xc\detail\frame_grabber_unit.hpp"
#include "xc\detail\frame_interval_property.hpp"
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

		frame_interval_property frame_interval() const
		{
			return frame_interval_property(this->state.handle(), this->state.unitmap(), 80, 10);
		}

		// 
		rxcpp::observable<image_view> when_image_captured() const
		{
			std::vector<HANDLE> wait_handles;
			for (const auto& unit : units)
				wait_handles.push_back(unit.capture_event().handle());
			//wait_handles.push_back(this->close_event.handle());
			return win32::when_any_event(wait_handles).map([this](int32_t index)
			{
				return this->units[index].get_last_captured_image();
			}).subscribe_on(rxcpp::observe_on_new_thread());
		}

		// 
		rxcpp::observable<fault_info> when_fault() const
		{
			std::vector<HANDLE> wait_handles;
			for (const auto& unit : units)
				wait_handles.push_back(unit.fault_event().handle());
			//wait_handles.push_back(this->close_event.handle());
			return win32::when_any_event(wait_handles).map([this](int32_t index)
			{
				return this->units[index].check_fault();
			}).subscribe_on(rxcpp::observe_on_new_thread());
		}
	private:
		detail::xclib_state state;
		std::vector<frame_grabber_unit> units;
		detail::high_resolution_system_time_point clock;
		//win32::manual_reset_event close_event;
	};
}