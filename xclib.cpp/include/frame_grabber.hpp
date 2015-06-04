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
#include "capture_observable.hpp"

namespace xc
{

	class frame_grabber_options
	{
	public:
		frame_grabber_options(const std::vector<int>& ports)
		{

		}

		std::vector<int> ports() const
		{

		}

	private:

	};
	namespace detail
	{
		void handle_result(int error)
		{
			if (error > 0)
				log_warn(::pxd_mesgErrorCode(error));
			else if (error < 0)
				throw std::runtime_error(std::string(::pxd_mesgErrorCode(error)));
		}

		namespace convert
		{
			std::string to_string(const frame_grabber_options& options)
			{
				int drivermap = std::accumulate(
					options.ports().begin(), 
					options.ports().end(), 
					0, 
					[](int drivermap, int next) { return drivermap + (1 << next); });

				return common::format("-DM %i", drivermap);
			}
		}
	}

	class frame_grabber
	{
	public:
		frame_grabber(
			const std::string& format_file_path,
			const frame_grabber_options& options)
		{
			this->open(format_file_path, options);
		}

		frame_grabber(const frame_grabber&) = delete;

		frame_grabber& operator=(const frame_grabber&) = delete;

		~frame_grabber()
		{
			try
			{
				close();
			}
			catch (const std::exception& ex)
			{
				log_error(ex.what());
			}
		}

		void open(
			const std::string& format_file_path,
			const frame_grabber_options& options)
		{
			this->instance = ::pxe_XCLIBinstantiate();
			if (instance == nullptr)
				throw std::runtime_error("pxe_XCLIBinstantiate failed");

			int result = ::pxe_PIXCIopen(
				this->instance,
				const_cast<char*>(detail::convert::to_string(options).c_str()),
				nullptr,
				const_cast<char*>(format_file_path.c_str()));
			detail::handle_result(result);
			this->check_fault(0);
//#include "VideoTriggerDisabled.fmt"
//			pxd_videoFormatAsIncluded(0);

			std::vector<detail::frame_grabber_unit_info> units;
			int index = 0;
			for (auto port : options.ports())
			{
				this->units.emplace_back(this->instance, port, port, 10);
				units.push_back(this->units[index].info());
				index++;
			}
			this->fault_observable_ptr = std::make_unique<detail::fault_observable>(units);
			this->capture_observable_ptr = std::make_unique<detail::capture_observable>(units);
		}

		void close()
		{
			int result = pxe_PIXCIclose(this->instance);
			detail::handle_result(result);

			::pxe_XCLIBuninstantiate(this->instance);
		}

		bool_t is_capturing()
		{
			auto result = pxe_goneLive(this->instance, this->map, 0);
			return result == 0 ? false : true;
		}

		// begins capture and enables strobe.
		void start_capture()
		{
//#include "VideoTriggerEnabled.fmt"
//			pxd_videoFormatAsIncluded(0);
			int result = pxe_goLiveSeq(this->instance, this->map, 1, 10, 1, 0, 1);
			detail::handle_result(result);
		}

		// ends capture and disables strobe.
		void stop_capture()
		{
//#include "VideoTriggerDisabled.fmt"
//			pxd_videoFormatAsIncluded(0);
			int result = pxe_goUnLive(this->instance, this->map);
			detail::handle_result(result);
		}

		rxcpp::observable<image> when_image_captured()
		{

		}

		rxcpp::observable<fault_info> when_fault()
		{
			return this->fault_observable_ptr->get();
		}

	private:

		void check_fault(int unitmap)
		{
			std::array<char, 1024> buffer;
			int result = ::pxe_mesgFaultText(this->instance, unitmap, buffer.data(), buffer.size());
			detail::handle_result(result);
			if (result == 1)
				throw std::runtime_error(std::string(buffer.data()));
		}

		std::vector<detail::frame_grabber_unit> units;
		std::unique_ptr<detail::fault_observable> fault_observable_ptr;
		std::unique_ptr<detail::capture_observable> capture_observable_ptr;
		pxdstate_s* instance;
		int map;
	};
}