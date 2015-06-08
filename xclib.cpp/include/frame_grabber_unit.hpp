#pragma once

#include "xcliball.h"

#include "frame_buffer_sequence.hpp"
#include "capture_event.hpp"
#include "fault_event.hpp"

#include <chrono>

#include "helpers.hpp"
#include "image_info.hpp"

namespace xc
{
	namespace detail
	{
		image_info from_frame_buffer(
			int port,
			const detail::frame_buffer& frame_buffer)
		{
			return image_info(
				port,
				frame_buffer.width(),
				frame_buffer.height(),
				frame_buffer.stride(),
				frame_buffer.step(),
				frame_buffer.data(),
				frame_buffer.get_timestamp());
		}

		// Manages resources for a single frame grabber unit.
		class frame_grabber_unit
		{
		public:
			// 
			frame_grabber_unit(pxdstate* state, int port, int map, size_t buffers) : 
				state(state),
				port_(port),
				map_(map),
				sequence(state, map, buffers),
				capture_event_(state, map),
				fault_event_(state, map)
			{

			}

			frame_grabber_unit(frame_grabber_unit&& other) :
				state(other.state), 
				port_(other.port_), 
				map_(other.map_), 
				sequence(std::move(other.sequence)),
				capture_event_(std::move(other.capture_event_)),
				fault_event_(std::move(other.fault_event_))
			{

			}

			frame_grabber_unit(const frame_grabber_unit&) = delete;

			frame_grabber_unit& operator=(const frame_grabber_unit&) = delete;

			// G
			const capture_event& capture_event() const
			{
				return this->capture_event_;
			}

			// Gets a windows event that is raised when a fault occurs.
			const fault_event& fault_event() const
			{
				return this->fault_event_;
			}

			// Gets the port this unit is on.
			int port() const
			{
				return this->port_;
			}

			// Gets the port mapping for this unit.
			int map() const
			{
				return this->map_;
			}

			// Returns access to the last captured image.
			image_info get_last_captured_image() const
			{
				pxbuffer_t buffer = ::pxe_capturedBuffer(this->state, this->map_);
				if (buffer <= 0)
					throw std::runtime_error("pxe_capturedBuffer");

				return from_frame_buffer(
					this->port_,
					this->sequence.get(buffer));
			}

			// Checks if capturing on this unit.
			bool_t is_capturing()
			{
				auto result = pxe_goneLive(this->state, this->map_, 0);
				return result == 0 ? false : true;
			}

			// Begins capture on this unit
			void start_capture()
			{
				int result = pxe_goLiveSeq(
					this->state, 
					this->map_, 
					1, static_cast<pxbuffer_t>(this->sequence.size()), 
					1, 0, 1);
				detail::handle_result(result);
			}

			// Ends capture on this unit
			void stop_capture()
			{
				int result = pxe_goUnLive(this->state, this->map_);
				detail::handle_result(result);
			}

			// Checks the unit for faults.  Throws an exception if a fault is detected.
			// NOTE: using exceptions for control flow makes me feel icky.
			void check_fault() const
			{
				std::array<char, 1024> buffer;
				int result = ::pxe_mesgFaultText(
					this->state, this->map_, 
					buffer.data(), buffer.size());
				if (result == 1)
					throw std::runtime_error(std::string(buffer.data()));
				else
					detail::handle_result(result);
			}
		private:
			pxdstate* const state;
			const int port_;
			const int map_;
			xc::detail::capture_event capture_event_;
			xc::detail::fault_event fault_event_;
			frame_buffer_sequence sequence;
		};
	}
}