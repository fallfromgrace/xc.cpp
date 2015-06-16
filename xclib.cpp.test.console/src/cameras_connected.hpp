#pragma once

#include <thread>
#include <vector>

#include "rx.hpp"
#include "xc\frame_grabber.hpp"
#include "more\string.hpp"
#include "win32\event.hpp"

namespace xc
{
	class cameras_connected
	{
	public:
		void capture()
		{
			std::vector<int> v = { 0, 1 };
			xc::frame_grabber fg(v, "C:\\Users\\Light\\Desktop\\video_trigger_disabled.fmt");

			//fg.when_image_captured().subscribe([](const image_view& image)
			//{
			//	printf(more::format("time: %s\n", more::to_string("%c", image.timestamp()).c_str()).c_str());
			//});

			fg.enable_strobe();
			fg.start_capture();

			rxcpp::observable<>::interval(
				std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::milliseconds(2000)),
				std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::milliseconds(330)))
				.subscribe_on(rxcpp::observe_on_new_thread())
				.map([&](long _)
			{
				return fg.get_unit(0).get_last_captured_image();
			}).subscribe([](const image_view& view)
			{
				printf("data: 0x%x | time: %s\n", view.data(), more::to_string("%c", view.timestamp()).c_str());
			},
				[](const std::exception_ptr& pex)
			{
				try
				{
					std::rethrow_exception(pex);
				}
				catch (const std::exception& ex)
				{
					printf(ex.what());
				}
			}, []()
			{
				printf("\ncompleted\n");
			});

			fg.frame_interval().set(1000000);
			std::this_thread::sleep_for(std::chrono::milliseconds(2500));

			fg.frame_interval().set(100000);
			std::this_thread::sleep_for(std::chrono::milliseconds(2500));

			fg.frame_interval().set(25000);
			std::this_thread::sleep_for(std::chrono::milliseconds(100000));
		}
	};
}