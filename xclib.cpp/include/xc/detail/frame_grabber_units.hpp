#pragma once

#include <unordered_map>

#include "xc\detail\frame_grabber_unit.hpp"

namespace xc
{
	class frame_grabber_units
	{
	public:
		frame_grabber_units(const std::vector<detail::port_info>& ports)
		{

		}

		const frame_grabber_unit& get(int port) const
		{
			return this->units[port];
		}
	private:
		std::unordered_map<int, frame_grabber_unit> units;
	};
}