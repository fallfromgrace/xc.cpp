#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>

#include "more\log.hpp"

#include "xc\detail\frame_grabber_options.hpp"
#include "xc\detail\util.hpp"

namespace xc
{
	namespace detail
	{
		// Manages an instance to xclib.
		class xclib_state
		{
		public:
			// 
			xclib_state(
				const std::vector<int>& ports,
				const std::string& format_file_path,
				const frame_grabber_options& options) :
				ports_(get_port_infos(ports)),
				drivermap(get_drivermap(ports)),
				format_file_path(format_file_path),
				options_(options),
				state_(nullptr),
				unitmap_(0)
			{
				this->open();
			}

			// 
			xclib_state(xclib_state&& other) :
				drivermap(other.drivermap),
				format_file_path(std::move(other.format_file_path)),
				options_(std::move(other.options_)),
				state_(other.state_),
				unitmap_(other.unitmap_)
			{
				other.state_ = nullptr;
				other.unitmap_ = 0;
			}

			// 
			~xclib_state()
			{
				try
				{
					this->close();
				}
				catch (const std::exception& ex)
				{
					more::error(ex.what());
				}
			}

			xclib_state(const xclib_state&) = delete;

			xclib_state& operator=(const xclib_state&) = delete;

			xclib_state& operator=(xclib_state&&) = delete;

			// Gets the mapping for all connected units.
			int unitmap() const
			{
				return this->unitmap_;
			}

			// 
			std::vector<port_info> ports() const
			{
				return this->ports_;
			}

			// Gets the allocated and initialized xclib state handle.
			pxdstate* handle()
			{
				return this->state_;
			}

			// Gets the allocated and initialized xclib state handle.
			const pxdstate* handle() const
			{
				return this->state_;
			}

			// 
			const frame_grabber_options& options() const
			{
				return this->options_;
			}

			// 
			bool_t is_open() const
			{
				return this->state_ != nullptr;
			}

			// 
			void open()
			{
				this->close();
				this->state_ = ::pxe_XCLIBinstantiate();
				if (this->state_ == nullptr)
					throw std::runtime_error("pxe_XCLIBinstantiate failed");

				int result = ::pxe_PIXCIopen(
					this->state_,
					const_cast<char*>(detail::get_driver_params(this->drivermap, this->options_).c_str()),
					nullptr,
					const_cast<char*>(this->format_file_path.c_str()));
				this->check_fault();
				detail::handle_result(result);
				for (int i = 0; i < this->ports_.size(); i++)
					this->unitmap_ += (1 << i);
			}

			// 
			void close()
			{
				if (is_open() == true)
				{
					int result = pxe_PIXCIclose(this->state_);
					detail::handle_result(result);

					::pxe_XCLIBuninstantiate(this->state_);
					this->state_ = nullptr;
					this->unitmap_ = 0;
				}
			}

			// Checks if a fault has occurred while openeing the xclib library.
			void check_fault()
			{
				std::array<char, 1024> buffer;
				int result = ::pxe_mesgFaultText(
					this->state_, this->unitmap_,
					buffer.data(), buffer.size());
				if (result == 1)
					throw std::runtime_error(std::string(buffer.data()));
				else
					detail::handle_result(result);
			}
		private:
			const std::vector<port_info> ports_;
			const int drivermap;
			const std::string format_file_path;
			const frame_grabber_options options_;
			pxdstate_s* state_;
			int unitmap_;
		};
	}
}