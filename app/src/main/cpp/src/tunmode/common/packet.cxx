#include <tunmode/common/packet.hpp>

#include <tunmode/common/utils.hpp>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <tunmode/definitions.hpp>

namespace tunmode
{
	Packet::Packet() : Buffer()
	{
		this->id = 0;
		this->protocol = 0;
	}

	uint64_t Packet::get_id() const
	{
		return this->id;
	}

	int Packet::get_protocol() const
	{
		return this->protocol;
	}

	InBuffer Packet::get_data() const
	{
		if (this->protocol == TUNMODE_PROTOCOL_TCP)
		{
			ip* ip_header;
			tcphdr* tcp_header;

			utils::point_headers_tcp((Packet*)this, &ip_header, &tcp_header);

			uintptr_t data_start = (uintptr_t)tcp_header + tcp_header->th_off * 4;

			return InBuffer((void*)data_start, (size_t)(this->size - (data_start - (uintptr_t)this->buffer)));
		}
		else if (this->protocol == TUNMODE_PROTOCOL_UDP)
		{
			ip* ip_header;
			udphdr* udp_header;

			utils::point_headers_udp((Packet*)this, &ip_header, &udp_header);

			uintptr_t data_start = (uintptr_t)(udp_header + 1);

			return InBuffer((void*)data_start, (size_t)(this->size - (data_start - (uintptr_t)this->buffer)));
		}

		return InBuffer();
	}

	void Packet::set_id(uint64_t id)
	{
		this->id = id;
	}

	void Packet::set_protocol(int protocol)
	{
		this->protocol = protocol;
	}
}