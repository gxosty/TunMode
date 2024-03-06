#pragma once

#include "session.hpp"
#include "../common/packet.hpp"
#include "../common/buffer.hpp"

#include <cstdint>

namespace tunmode
{
	class UDPManager;

	class UDPSession : public Session
	{
	public:
		UDPSession(UDPManager* manager, uint64_t id);

	private:
		UDPManager* manager;
		uint32_t poll_timeout;

		int poll(struct pollfd fds[2]) override;
		void loop() override;
		void _loop();
	};
}