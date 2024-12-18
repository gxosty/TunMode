#pragma once

#include "socket.hpp"
#include "sessionsocket.hpp"
#include "../common/packet.hpp"
#include "../common/buffer.hpp"

#include <netinet/in.h>
#include <cstdint>

namespace tunmode
{
	class UDPSocket : public SessionSocket
	{
	public:
		UDPSocket();
		~UDPSocket() override;

		void init(Socket* skt);

		size_t send_tun(Packet& packet) override;
		size_t send(const Buffer& buffer) override;
		size_t recv(Buffer& buffer) override;

		void operator<<(const Buffer& buffer) override;
		void operator>>(Buffer& buffer) override;

		void close();

	private:
		in_addr client_addr;
		u_short client_port;
		in_addr server_addr;
		u_short server_port;
	};
}