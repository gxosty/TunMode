#pragma once

#include "../common/packet.hpp"
#include "sessionmanager.hpp"

namespace tunmode
{
	class TCPSession;
	
	class TCPManager : public SessionManager
	{
	public:
		TCPManager();

		void handle_packet(const Packet& packet) override;

	private:
		Session* add(uint64_t id) override;

		friend class TCPSession;
	};
}