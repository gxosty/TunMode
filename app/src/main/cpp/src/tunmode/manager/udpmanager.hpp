#pragma once

#include "../common/packet.hpp"
#include "sessionmanager.hpp"

namespace tunmode
{
	class UDPSession;

	class UDPManager : public SessionManager
	{
	public:
		UDPManager();

		void handle_packet(const Packet& packet) override;

	private:
		Session* add(uint64_t id) override;

		friend class UDPSession;
	};
}