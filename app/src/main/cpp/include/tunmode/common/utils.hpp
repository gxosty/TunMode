#pragma once

#include "packet.hpp"

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <cstdint>

namespace tunmode::utils
{
	uint64_t make_tcp_id(Packet* packet);
	uint64_t make_udp_id(Packet* packet);

	void     build_tcp_packet(Packet* packet);

	void     point_headers_tcp(const Packet* packet, ip** ip_header, tcphdr** tcp_header);

	void     finalize_packet_tcp(Packet* packet);

	void     protect_socket(int skt);

	void     print_packet_tcp(Packet* packet);
}