#include <tunmode/socket/udpsocket.hpp>
#include <tunmode/common/utils.hpp>
#include <tunmode/common/inbuffer.hpp>
#include <tunmode/definitions.hpp>

#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <misc/logger.hpp>

namespace tunmode
{
	UDPSocket::UDPSocket() : SessionSocket() {}

	UDPSocket::~UDPSocket()
	{
		SessionSocket::~SessionSocket();
	}

	void UDPSocket::init(Socket* skt)
	{
		Packet client_packet;
		client_packet.set_protocol(TUNMODE_PROTOCOL_UDP);
		ip* ip_header;
		udphdr* udp_header;

		*this > client_packet;

		utils::point_headers_udp(&client_packet, &ip_header, &udp_header);

		this->client_addr = ip_header->ip_src;
		this->client_port = udp_header->uh_sport;
		this->server_addr = ip_header->ip_dst;
		this->server_port = udp_header->uh_dport;

		in_addr sv_addr;
		inet_pton(AF_INET, "192.168.1.101", &sv_addr);
		skt->bind(sv_addr, 0);
		skt->connect(this->server_addr, this->server_port);

		InBuffer in_buffer = client_packet.get_data();
		Buffer buffer(in_buffer.get_buffer(), in_buffer.get_size());
		*skt << buffer;
	}

	size_t UDPSocket::send_tun(Packet& packet)
	{
		utils::finalize_packet_udp(&packet);
		return SessionSocket::send_tun(packet);
	}

	size_t UDPSocket::send(const Buffer& buffer)
	{
		Packet packet;
		packet.set_protocol(TUNMODE_PROTOCOL_UDP);
		ip* ip_header;
		udphdr* udp_header;

		utils::build_udp_packet(&packet);
		utils::point_headers_udp(&packet, &ip_header, &udp_header);

		ip_header->ip_src = this->server_addr;
		udp_header->uh_sport = this->server_port;
		ip_header->ip_dst = this->client_addr;
		udp_header->uh_dport = this->client_port;

		InBuffer in_buffer = packet.get_data();
		memcpy(in_buffer.get_buffer(), buffer.get_buffer(), buffer.get_size());
		packet.set_size(packet.get_size() + buffer.get_size());

		return this->send_tun(packet);
	}

	size_t UDPSocket::recv(Buffer& buffer)
	{
		Packet packet;
		packet.set_protocol(TUNMODE_PROTOCOL_UDP);
		ip* ip_header;
		udphdr* udp_header;

		*this > packet;

		// utils::point_headers_udp(&packet, &ip_header, &udp_header);

		InBuffer in_buffer = packet.get_data();
		buffer(in_buffer.get_buffer(), in_buffer.get_size());
		return buffer.get_size();
	}

	void UDPSocket::operator<<(const Buffer& buffer)
	{
		this->send(buffer);
	}

	void UDPSocket::operator>>(Buffer& buffer)
	{
		this->recv(buffer);
	}

	void UDPSocket::close()
	{
		return;
	}
}