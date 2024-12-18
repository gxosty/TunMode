#pragma once

#include "../common/buffer.hpp"

#include <netinet/in.h>
#include <cstdint>

namespace tunmode
{
	class Socket
	{
	public:
		Socket(int domain, int type, int protocol = 0);
		Socket(int socket);
		~Socket();

		int    bind(in_addr addr, u_short port);

		int    connect(in_addr addr, u_short port);
		int    close();

		size_t send(const Buffer* buffer, int flags = 0);
		size_t recv(Buffer* buffer, int flags = 0);

		int    get_socket();
		int    set_nonblocking(bool state);

		void   operator<<(const Buffer& buffer);
		void   operator<<(const Buffer* buffer);
		void   operator>>(Buffer& buffer);
		void   operator>>(Buffer* buffer);

	private:
		int socket;
		bool closed;
	};
}