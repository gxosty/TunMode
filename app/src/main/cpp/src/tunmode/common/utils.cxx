#include <tunmode/common/utils.hpp>

namespace tunmode::utils
{
	uint64_t make_tcp_id(Packet* packet)
	{
		struct ip* ip_header = (struct ip*)packet->get_buffer();
		struct tcphdr* tcp_header = (struct tcphdr*)((uintptr_t)packet->get_buffer() + ip_header->ip_hl * 4);
		uint64_t id = ((uint64_t)ip_header->ip_dst.s_addr << 32) | ((uint64_t)tcp_header->th_dport << 16) | ((uint64_t)tcp_header->th_sport);
		packet->set_id(id);
		return id;
	}

	uint64_t make_udp_id(Packet* packet)
	{
		struct ip* ip_header = (struct ip*)packet->get_buffer();
		struct udphdr* udp_header = (struct udphdr*)((uintptr_t)packet->get_buffer() + ip_header->ip_hl * 4);
		uint64_t id = ((uint64_t)ip_header->ip_dst.s_addr << 32) | ((uint64_t)udp_header->uh_dport << 16) | ((uint64_t)udp_header->uh_sport);
		packet->set_id(id);
		return id;
	}
}