#pragma once

#include <string>

namespace xc
{
	// 
	class fault_info
	{
	public:
		// 
		fault_info(int port, const std::string& message) :
			port_(port), message(message)
		{

		}

		// 
		int port() const
		{
			return this->port_;
		}

		// 
		const std::string& what() const
		{
			return this->message;
		}

	private:

		int port_;
		std::string message;
	};
}