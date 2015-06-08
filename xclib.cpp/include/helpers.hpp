#pragma once

#include <bitset>
#include <limits>
#include <stdexcept>

#include "logging\logging.hpp"
#include "xcliball.h"

#include "frame_grabber_options.hpp"

namespace xc
{
	namespace detail
	{
		void handle_result(int result)
		{
			if (result > 0)
				log_warn(std::string(::pxd_mesgErrorCode(result)));
			else if (result < 0)
				throw std::runtime_error(std::string(::pxd_mesgErrorCode(result)));
		}

		class port_info
		{
		public:
			port_info(int port, int map) :
				port_(port), map_(map)
			{

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
			const int port_;
			const int map_;
		};

		std::vector<port_info> get_port_infos(const std::vector<int>& ports)
		{
			std::vector<port_info> ports_infos;

			std::vector<int> ports_copy(ports);
			std::sort(ports_copy.begin(), ports_copy.end());
			for (size_t i = 0; i < ports_copy.size(); i++)
				ports_infos.emplace_back(ports_copy[i], 1 << i);
			return ports_infos;
		}

		int get_drivermap(const std::vector<int>& ports)
		{
			return std::accumulate(
				ports.begin(),
				ports.end(),
				0,
				[](int drivermap, int next) { return drivermap + (1 << next); });
		}

		template<typename value_type>
		std::bitset<std::numeric_limits<value_type>::digits> make_bitset(value_type v)
		{
			return std::bitset<std::numeric_limits<value_type>::digits>(v);
		}

		std::vector<int> get_ports(int drivermap)
		{
			std::vector<int> ports;
			auto drivermap_bits = make_bitset(drivermap);
			for (std::size_t i = 0; i < drivermap_bits.size(); ++i)
				if (drivermap_bits[i] == true)
					ports.push_back(static_cast<int>(i));
			return ports;
		}

		std::string get_driver_params(int drivermap, const frame_grabber_options& options)
		{
			return common::format("-DM 0x%x", drivermap);
		}
	}
}