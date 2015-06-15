#pragma once

namespace xc
{
	namespace detail
	{
		// Mapping between a port and the unit id.
		class port_map
		{
		public:
			port_map(int port, int map) :
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
	}
}