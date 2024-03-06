#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/common/utils.hpp>
#include <tunmode/definitions.hpp>

#include <unistd.h>
#include <netinet/ip.h>

#include <misc/logger.hpp>

namespace tunmode
{
	TunSocket* SessionSocket::tun = nullptr;

	SessionSocket::SessionSocket()
	{
		this->session_pipe[0] = 0;
		this->session_pipe[1] = 0;

		pipe2(this->session_pipe, O_DIRECT);
	}

	SessionSocket::~SessionSocket()
	{
		::close(this->session_pipe[0]);
		::close(this->session_pipe[1]);
	}

	size_t SessionSocket::send_tun(Packet& packet)
	{
		return SessionSocket::tun->send(&packet);
	}

	size_t SessionSocket::send(const Packet& packet)
	{
		return ::write(this->session_pipe[1], packet.get_buffer(), packet.get_size());
	}

	size_t SessionSocket::recv(Packet& packet)
	{
		size_t size = ::read(this->session_pipe[0], packet.get_buffer(), TUNMODE_BUFFER_SIZE);
		packet.set_size(size);
		ip* ip_header = (ip*)packet.get_buffer();
		int proto = ip_header->ip_p;
		packet.set_protocol(proto);

		switch (proto)
		{
		case (TUNMODE_PROTOCOL_TCP):
			utils::make_tcp_id(&packet);
			break;
		case (TUNMODE_PROTOCOL_UDP):
			utils::make_udp_id(&packet);
			break;
		default:
			packet.set_protocol(TUNMODE_PROTOCOL_UNKNOWN);
			break;
		}

		return size;
	}

	void SessionSocket::operator<(const Packet& packet)
	{
		this->send(packet);
	}

	void SessionSocket::operator>(Packet& packet)
	{
		this->recv(packet);
	}

	int SessionSocket::get_read_pipe()
	{
		return this->session_pipe[0];
	}
}