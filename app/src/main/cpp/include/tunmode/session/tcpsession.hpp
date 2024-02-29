#pragma once

#include "session.hpp"

#include <cstdint>

namespace tunmode
{
	class TCPSession : public Session
	{
	public:
		TCPSession(uint64_t id);
		~TCPSession() override;

	private:
		int  poll() override;
		void loop() override;
	};
}