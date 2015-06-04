#pragma once

#include "xcliball.h"

#include "frame_buffer_sequence.hpp"
#include "capture_event.hpp"
#include "fault_event.hpp"

#include <chrono>

namespace xc
{
	namespace detail
	{

		class frame_grabber_unit_info
		{
		public:
			frame_grabber_unit_info(
				HANDLE capture_event_handle,
				HANDLE fault_event_handle, 
				int port, 
				int map) :
				capture_event_handle_(capture_event_handle),
				fault_event_handle_(fault_event_handle),
				port_(port),
				map_(map)
			{

			}

			HANDLE fault_event_handle() const
			{
				return this->fault_event_handle_;
			}

			HANDLE capture_event_handle() const
			{
				return this->capture_event_handle_;
			}

			int port() const
			{
				return this->port_;
			}

			int map() const
			{
				return this->map_;
			}

		private:
			HANDLE fault_event_handle_;
			HANDLE capture_event_handle_;
			int port_;
			int map_;
		};

		class image
		{
		public:
			image(
				const frame_buffer_info& frame_buffer_info,
				uint32 timestamp_buffer[2]) : 
				frame_buffer_info(frame_buffer_info),
				time_point(std::chrono::system_clock::from_time_t(*reinterpret_cast<time_t*>(timestamp_buffer)))
			{

			}

			const std::chrono::system_clock::time_point& timestamp() const
			{
				return this->time_point;
			}

			const frame_buffer_info& info() const
			{
				return this->frame_buffer_info;
			}
		private:
			frame_buffer_info frame_buffer_info;
			std::chrono::system_clock::time_point time_point;
		};

		class frame_grabber_unit
		{
		public:
			frame_grabber_unit(pxdstate* state, int port, int unitmap, int buffers) : 
				sequence(state, unitmap, buffers),
				capture_event(state, unitmap),
				fault_event(state, unitmap)
			{

			}

			frame_grabber_unit(const frame_grabber_unit&) = delete;

			frame_grabber_unit& operator=(const frame_grabber_unit&) = delete;

			frame_grabber_unit_info info() const
			{
				return frame_grabber_unit_info(
					this->capture_event.handle(),
					this->fault_event.handle(),
					this->port,
					this->map);
			}

			// Returns access to the last captured image.
			image last_captured_image() const
			{
				pxbuffer_t buffer = ::pxe_capturedBuffer(this->state, this->map);
				if (buffer <= 0)
					throw std::runtime_error("pxe_capturedBuffer");

				uint32 timestamp_buffer[2];
				int result = ::pxe_buffersSysTicks2(this->state, this->map, buffer, timestamp_buffer);
				handle_result(result);

				return image(
					this->sequence.get(buffer),
					timestamp_buffer);
			}

			bool_t is_capturing()
			{
				auto result = pxe_goneLive(this->state, map, 0);
				return result == 0 ? false : true;
			}

			// begins capture and enables strobe.
			void start_capture()
			{
				int result = pxe_goLiveSeq(this->state, map, 1, 10, 1, 0, 1);
				detail::handle_result(result);
			}

			// ends capture and disables strobe.
			void stop_capture()
			{
				int result = pxe_goUnLive(this->state, map);
				detail::handle_result(result);
			}

		private:
			pxdstate* state;
			int port;
			int map;
			capture_event capture_event;
			fault_event fault_event;
			frame_buffer_sequence sequence;
		};
	}
}