#include <tunmode/common/utils.hpp>
#include <tunmode/tunmode.hpp>
#include <tunmode/definitions.hpp>
#include <RawSocket/CheckSum.h>

#include <arpa/inet.h>
#include <jni.h>
#include <random>

#include <misc/logger.hpp>

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

	void build_tcp_packet(Packet* packet)
	{
		struct ip* ip_header = (struct ip*)packet->get_buffer();
		struct tcphdr* tcp_header = (struct tcphdr*)((uintptr_t)packet->get_buffer() + 20);

		ip_header->ip_hl = 5;
		ip_header->ip_v = 4;
		ip_header->ip_len = htons(40);
		ip_header->ip_id = (uint16_t)rand();
		ip_header->ip_off = 0;
		ip_header->ip_ttl = 128;
		ip_header->ip_p = TUNMODE_PROTOCOL_TCP;

		tcp_header->th_seq = 0;
		tcp_header->th_ack = 0;
		tcp_header->th_off = 5;
		tcp_header->th_x2 = 0;
		tcp_header->th_flags = 0;
		tcp_header->th_win = htons(512);
		tcp_header->th_urp = 0;

		packet->set_size(40);
	}

	void build_udp_packet(Packet* packet)
	{
		struct ip* ip_header = (struct ip*)packet->get_buffer();
		struct udphdr* udp_header = (struct udphdr*)((uintptr_t)packet->get_buffer() + 20);

		ip_header->ip_hl = 5;
		ip_header->ip_v = 4;
		ip_header->ip_len = htons(28);
		ip_header->ip_id = (uint16_t)rand();
		ip_header->ip_off = 0;
		ip_header->ip_ttl = 128;
		ip_header->ip_p = TUNMODE_PROTOCOL_UDP;

		udp_header->uh_ulen = 0;
		udp_header->uh_sum = 0;

		packet->set_size(28);
	}

	void point_headers_tcp(const Packet* packet, ip** ip_header, tcphdr** tcp_header)
	{
		*ip_header = (ip*)(packet->get_buffer());
		*tcp_header = (tcphdr*)(ip_header[0]->ip_hl * 4 + (uintptr_t)packet->get_buffer());
	}

	void point_headers_udp(const Packet* packet, ip** ip_header, udphdr** udp_header)
	{
		*ip_header = (ip*)(packet->get_buffer());
		*udp_header = (udphdr*)(ip_header[0]->ip_hl * 4 + (uintptr_t)packet->get_buffer());
	}

	void finalize_packet_tcp(Packet* packet)
	{
		ip* ip_header;
		tcphdr* tcp_header;
		point_headers_tcp(packet, &ip_header, &tcp_header);

		ip_header->ip_len = htons(packet->get_size());
		ip_header->ip_sum = cksumIp((struct iphdr*)ip_header);
		tcp_header->th_sum = cksumTcp((struct iphdr*)ip_header, tcp_header);
	}

	void finalize_packet_udp(Packet* packet)
	{
		ip* ip_header;
		udphdr* udp_header;
		point_headers_udp(packet, &ip_header, &udp_header);

		InBuffer in_buffer = packet->get_data();

		ip_header->ip_len = htons(packet->get_size());
		ip_header->ip_sum = cksumIp((struct iphdr*)ip_header);
		udp_header->uh_ulen = htons(in_buffer.get_size() + 8);
		udp_header->uh_sum = cksumUdp((struct iphdr*)ip_header, udp_header);
	}

	/* Call only in one thread */
	void protect_socket(int skt)
	{
		JNIEnv* env;
		int env_status = tunmode::get_jni_env(&env);

		if (env_status == 2)
			return;

		static jclass    VpnService_class = env->FindClass("android/net/VpnService");
		static jmethodID VpnService_protect_methodID = env->GetMethodID(
			VpnService_class,
			"protect",
			"(I)Z"
		);

		env->CallBooleanMethod(
			tunmode::params::TunModeService_object,
			VpnService_protect_methodID,
			skt
		);
	}

	void print_packet_tcp(Packet* packet)
	{
#ifndef RELEASE_MODE
		ip* ip_header;
		tcphdr* tcp_header;
		point_headers_tcp(packet, &ip_header, &tcp_header);

		LOGD_("------[Packet]------");
		LOGD_("--------[IP]--------");

		LOGD_("ip_hl: %u",  (unsigned)ip_header->ip_hl);
		LOGD_("ip_v: %u",   (unsigned)ip_header->ip_v);
		LOGD_("ip_tos: %u", (unsigned)ip_header->ip_tos);
		LOGD_("ip_len: %u", (unsigned)ntohs(ip_header->ip_len));
		LOGD_("ip_id: %u",  (unsigned)ntohs(ip_header->ip_id));
		LOGD_("ip_off: %u", (unsigned)ntohs(ip_header->ip_off));
		LOGD_("ip_ttl: %u", (unsigned)ip_header->ip_ttl);
		LOGD_("ip_p: %u",   (unsigned)ip_header->ip_p);
		LOGD_("ip_sum: %u", (unsigned)ntohs(ip_header->ip_sum));
		LOGD_("ip_src: %s", inet_ntoa(ip_header->ip_src));
		LOGD_("ip_dst: %s", inet_ntoa(ip_header->ip_dst));

		LOGD_("-------[TCP]--------");

		LOGD_("th_sport: %u", (unsigned)ntohs(tcp_header->th_sport));
		LOGD_("th_dport: %u", (unsigned)ntohs(tcp_header->th_dport));
		LOGD_("th_seq: %u",   (unsigned)tcp_header->th_seq);
		LOGD_("th_ack: %u",   (unsigned)tcp_header->th_ack);
		LOGD_("th_off: %u",   (unsigned)tcp_header->th_off);
		LOGD_("th_flags: %u", (unsigned)tcp_header->th_flags);
		LOGD_("th_win: %u",   (unsigned)ntohs(tcp_header->th_win));
		LOGD_("th_sum: %u",   (unsigned)ntohs(tcp_header->th_seq));

		LOGD_("--------------------");
#endif
	}

	void print_packet_udp(Packet* packet)
	{

	}
}