#include <tunmode/socket/tunsocket.hpp>
#include <tunmode/common/utils.hpp>
#include <tunmode/definitions.hpp>

#include <unistd.h>
#include <netinet/ip.h>

namespace tunmode
{
	TunSocket::TunSocket()
	{
		this->tunnel = 0;
	}

	void TunSocket::close()
	{
		::close(this->tunnel);
	}

	size_t TunSocket::send(const Packet* packet)
	{
		return ::write(this->tunnel, packet->get_buffer(), packet->get_size());
	}

	size_t TunSocket::recv(Packet* packet)
	{
		size_t size = ::read(this->tunnel, packet->get_buffer(), packet->get_size());
		packet->set_size(size);
		ip* ip_header = (ip*)packet->get_buffer();
		int proto = ip_header->ip_p;
		packet->set_protocol(proto);

		switch (proto)
		{
		case (TUNMODE_PROTOCOL_TCP):
			utils::make_tcp_id(packet);
			break;
		case (TUNMODE_PROTOCOL_UDP):
			utils::make_udp_id(packet);
			break;
		default:
			packet->set_protocol(TUNMODE_PROTOCOL_UNKNOWN);
			break;
		}

		return size;
	}

	int TunSocket::poll(int timeout, int& revents)
	{
		this->fds[0].fd = this->tunnel;
		this->fds[0].events = POLLIN;
		this->fds[0].revents = 0;

		int ret = ::poll(this->fds, 1, timeout);
		revents = this->fds[0].revents;

		return ret;
	}

	int TunSocket::get_tunnel()
	{
		return this->tunnel;
	}

	void TunSocket::operator<<(const Packet& packet)
	{
		this->send(&packet);
	}

	void TunSocket::operator>>(Packet& packet)
	{
		this->recv(&packet);
	}

	int TunSocket::operator=(const int& tunnel)
	{
		return this->tunnel = tunnel;
	}
}