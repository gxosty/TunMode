#pragma once

#include "../common/packet.hpp"

#include <netinet/in.h>
#include <poll.h>
#include <cstdint>

namespace tunmode
{
	class TunSocket
	{
	public:
		TunSocket();

		void   close();

		size_t send(const Packet* packet);
		size_t recv(Packet* packet);

		int    poll(int timeout, int& revents);

		int    get_tunnel();

		void   operator<(const Packet& packet);
		void   operator>(Packet& packet);
		int    operator=(const int& tunnel);

	private:
		int tunnel;
		struct pollfd fds[1];
	};
}