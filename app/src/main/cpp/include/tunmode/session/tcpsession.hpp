#pragma once

#include "session.hpp"
#include "../common/packet.hpp"
#include "../common/buffer.hpp"

#include <cstdint>

namespace tunmode
{
	class TCPManager;

	class TCPSession : public Session
	{
	public:
		TCPSession(TCPManager* manager, uint64_t id);

	private:
		TCPManager* manager;

		int  poll(struct pollfd fds[2]) override;
		void loop() override;
		void _loop();

		int  poll2(struct pollfd fds[2], Buffer& client_buffer, Buffer& server_buffer);
	};
}