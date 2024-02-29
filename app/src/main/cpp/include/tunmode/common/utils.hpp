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
}