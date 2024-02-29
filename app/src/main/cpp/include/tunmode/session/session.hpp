#pragma once

#include "../socket/socket.hpp"
#include "../socket/sessionsocket.hpp"

#include <cstdint>

namespace tunmode
{
	class Session
	{
	public:
		static in_addr iface_addr;

		Session(uint64_t id);
		virtual ~Session();

		uint64_t get_id();
		SessionSocket* get_client_socket();

	protected:
		uint64_t id;

		SessionSocket* client_socket;
		Socket* server_socket;

		virtual int poll() = 0;
		virtual void loop() = 0;
	};
}