#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <Windows.h>

#include "includes.hpp"
#include "common\string_format.hpp"
#include "logging\logging.hpp"
#include "xcliball.h"

#include <array>

#include <numeric>
#include "rx.hpp"

#include "frame_grabber_unit.hpp"

#include "fault_observable.hpp"
#include "frame_grabber_options.hpp"
#include "capture_observable.hpp"
#include "helpers.hpp"
#include "driver.hpp"

namespace xc
{
	// Manages resources for a set of frame grabber units
	class frame_grabber
	{
	public:
		// 
		frame_grabber(
			const std::vector<int>& ports,
			const std::string& format_file_path,
			const frame_grabber_options& options = frame_grabber_options()) :
			driver(ports, format_file_path, options)
		{
			for (const auto& port_info : this->driver.ports())
				this->units.push_back(std::make_unique<detail::frame_grabber_unit>(
					this->driver.state(),
					port_info.port(),
					port_info.map(),
					this->driver.options().buffer_count()));
			this->fault_observable_ptr = std::make_unique<detail::fault_observable>(this->units);
			this->capture_observable_ptr = std::make_unique<detail::capture_observable>(this->units);
		}

		frame_grabber(frame_grabber&& other) : 
			driver(std::move(other.driver)),
			units(std::move(other.units)),
			fault_observable_ptr(std::move(other.fault_observable_ptr)),
			capture_observable_ptr(std::move(other.capture_observable_ptr))
		{
			
		}

		frame_grabber(const frame_grabber&) = delete;

		frame_grabber& operator=(const frame_grabber&) = delete;

		// 
		bool_t is_capturing()
		{
			auto result = pxe_goneLive(this->driver.state(), this->driver.unitmap(), 0);
			return result == 0 ? false : true;
		}

		// Begins capture
		void start_capture()
		{
			if (this->is_capturing() == false)
			{
				int result = pxe_goLiveSeq(
					this->driver.state(),
					this->driver.unitmap(),
					1, static_cast<pxbuffer_t>(this->driver.options().buffer_count()),
					1, 0, 1);
				detail::handle_result(result);
			}
		}

		// Ends capture
		void stop_capture()
		{
			if (this->is_capturing() == true)
			{
				int result = pxe_goUnLive(this->driver.state(), this->driver.unitmap());
				detail::handle_result(result);
			}
		}

		void enable_strobe()
		{
			#include "VideoTriggerEnabled.fmt"
			pxe_videoFormatAsIncludedInit(this->driver.state(), 0);
			int result = pxe_videoFormatAsIncluded(this->driver.state(), 0);
			detail::handle_result(result);
		}

		void disable_strobe()
		{
			#include "VideoTriggerDisabled.fmt"
			pxe_videoFormatAsIncludedInit(this->driver.state(), 0);
			int result = pxe_videoFormatAsIncluded(this->driver.state(), 0);
			detail::handle_result(result);
		}

		rxcpp::observable<image_info> when_image_captured() const
		{
			return this->capture_observable_ptr->get();
		}

		rxcpp::observable<fault_info> when_fault() const
		{
			return this->fault_observable_ptr->get();
		}
	private:
		detail::driver driver;
		std::vector<std::unique_ptr<detail::frame_grabber_unit>> units;
		std::unique_ptr<detail::fault_observable> fault_observable_ptr;
		std::unique_ptr<detail::capture_observable> capture_observable_ptr;
	};
}