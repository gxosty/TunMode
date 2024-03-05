#pragma once

#include "tunsocket.hpp"
#include "../common/packet.hpp"

#include <netinet/in.h>
#include <cstdint>

namespace tunmode
{
	class SessionSocket
	{
	public:
		static TunSocket* tun;

		SessionSocket();
		virtual ~SessionSocket();

		virtual size_t send(const Packet& packet);
		virtual size_t send(const Buffer& buffer) = 0;
		virtual size_t recv(Packet& packet);
		virtual size_t recv(Buffer& buffer) = 0;

		virtual void   operator<(const Packet& packet);
		virtual void   operator<<(const Buffer& buffer) = 0;
		virtual void   operator>(Packet& packet);
		virtual void   operator>>(Buffer& buffer) = 0;

		int get_read_pipe();

	private:
		int session_pipe[2];
	};
}