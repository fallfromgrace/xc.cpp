#pragma once

#include <chrono>

#include "xcliball.h"

#include "xc\detail\capture_event.hpp"
#include "xc\detail\image_view.hpp"
#include "xc\detail\fault_event.hpp"
#include "xc\detail\fault_info.hpp"
#include "xc\detail\high_resolution_system_time_point.hpp"
#include "xc\detail\frame_buffer_sequence.hpp"
#include "xc\detail\util.hpp"

namespace xc
{
	// Allocates and manages resources for a single frame grabber unit.
	class frame_grabber_unit
	{
	public:
		// 
		frame_grabber_unit(
			pxdstate* state,
			const xc::detail::port_info& port_info,
			size_t buffers,
			const detail::high_resolution_system_time_point& clock) :
			state(state),
			port_(port_info.port()),
			map_(port_info.map()),
			fault_event_(state, port_info.map()),
			capture_event_(state, port_info.map()),
			sequence(state, port_info.map(), buffers),
			clock(clock)
		{

		}

		// 
		frame_grabber_unit(frame_grabber_unit&& other) :
			state(other.state),
			port_(other.port_),
			map_(other.map_),
			clock(other.clock),
			fault_event_(std::move(other.fault_event_)),
			capture_event_(std::move(other.capture_event_)),
			sequence(std::move(other.sequence))
		{

		}

		//// 
		//frame_grabber_unit& operator=(frame_grabber_unit&& other)
		//{
		//	return *this;
		//}

		frame_grabber_unit(const frame_grabber_unit&) = delete;

		frame_grabber_unit& operator=(const frame_grabber_unit&) = delete;

		// Creates a win32 event that signals when a new image is captured into an allocated frame
		// buffer.  This image can be retrieved via get_last_captured_image.
		const capture_event& capture_event() const
		{
			return this->capture_event_;
		}

		// Creates a win32 event that signals when a fault occurs in the hardware.  Details of this 
		// fault can be retrieved via check_fault.
		const fault_event& fault_event() const
		{
			return this->fault_event_;
		}

		// Returns a view of the last captured image.
		image_view get_last_captured_image() const
		{
			pxbuffer_t id = ::pxe_capturedBuffer(this->state, this->map_);
			if (id <= 0)
				throw std::runtime_error("pxe_capturedBuffer");

			return image_view(this->port_, this->sequence.get(id), this->clock);
		}

		// Gets access to the frame buffer sequence.
		const frame_buffer_sequence& get_sequence() const
		{
			return this->sequence;
		}

		// Gets the port this unit is on.
		int port() const
		{
			return this->port_;
		}

		// Checks if capturing on this unit.
		bool_t is_capturing()
		{
			auto result = pxe_goneLive(this->state, this->map_, 0);
			return result == 0 ? false : true;
		}

		// Begins capture on this unit.
		void start_capture()
		{
			int result = pxe_goLiveSeq(
				this->state,
				this->map_,
				1, static_cast<pxbuffer_t>(this->sequence.size()),
				1, 0, 1);
			detail::handle_result(result);
		}

		// Ends capture on this unit.
		void stop_capture()
		{
			int result = pxe_goUnLive(this->state, this->map_);
			detail::handle_result(result);
		}

		// Checks the unit for faults.  Throws an exception if a fault is detected.
		// NOTE: using exceptions for control flow makes me feel icky.
		fault_info check_fault() const
		{
			std::array<char, 1024> buffer;
			int result = ::pxe_mesgFaultText(
				this->state, this->map_,
				buffer.data(), buffer.size());
			if (result == 1)
				return fault_info(this->port_, std::string(buffer.data()));
			else
				detail::handle_result(result);
			return fault_info(this->port_, "No fault detected.");
		}
	private:
		pxdstate* const state;
		const int port_;
		const int map_;
		frame_buffer_sequence sequence;
		xc::fault_event fault_event_;
		xc::capture_event capture_event_;
		detail::high_resolution_system_time_point clock;

	};
}