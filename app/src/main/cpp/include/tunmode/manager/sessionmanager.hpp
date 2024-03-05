#pragma once

#include "../session/session.hpp"
#include "../common/packet.hpp"

#include <unordered_map>
#include <cstdint>

namespace tunmode
{
	class SessionManager
	{
	public:
		SessionManager();

		virtual void handle_packet(const Packet& packet) = 0;

	protected:
		std::unordered_map<uint64_t, Session*> sessions;

		virtual Session* add(uint64_t id) = 0;
		Session* get_or_add(uint64_t id);
		void remove(uint64_t id);
	};
}