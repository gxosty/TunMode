#include <tunmode/socket/tcpsocket.hpp>
#include <tunmode/common/utils.hpp>
#include <tunmode/definitions.hpp>
#include <tunmode/tunmode.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <random>

#include <misc/logger.hpp>

namespace tunmode
{
	TCPSocket::TCPSocket() : SessionSocket()
	{
		this->state = TCPSTATE_LISTEN;
		this->syn_recved = false;
	}

	TCPSocket::~TCPSocket()
	{
		SessionSocket::~SessionSocket();
	}

	int TCPSocket::connect(Socket* skt)
	{
		static constexpr char hs_opts[] = {2, 4, 15, 160, 1, 3, 3, 8, 1, 1, 4, 2};

		Packet client_packet;
		client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
		Packet server_packet;
		server_packet.set_protocol(TUNMODE_PROTOCOL_TCP);

		*this > client_packet;

		ip* ip_header;
		tcphdr* tcp_header;

		utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

		this->client_addr = ip_header->ip_src;
		this->client_port = tcp_header->th_sport;
		this->server_addr = ip_header->ip_dst;
		this->server_port = tcp_header->th_dport;

		this->vars.rcv.irs = ntohl(tcp_header->th_seq);
		this->vars.rcv.nxt = this->vars.rcv.irs + 1;
		this->vars.rcv.wnd = TUNMODE_BUFFER_SIZE;
		this->vars.snd.wnd = ntohs(tcp_header->th_win);
		this->vars.snd.iss = (uint32_t)rand();
		this->vars.snd.nxt = this->vars.snd.iss + 1;

		if (tcp_header->th_flags != TH_SYN)
		{
			this->set_state(TCPSTATE_CLOSED);
			this->reset(client_packet);
			return -1;
		}

		this->set_state(TCPSTATE_SYN_RECEIVED);

		skt->bind(params::net_iface, 0);

		if (skt->connect(this->server_addr, this->server_port) == -1)
		{
			this->reset(client_packet);
			return -1;
		}

		utils::build_tcp_packet(&server_packet);
		utils::point_headers_tcp(&server_packet, &ip_header, &tcp_header);

		ip_header->ip_src = this->server_addr;
		tcp_header->th_sport = this->server_port;
		ip_header->ip_dst = this->client_addr;
		tcp_header->th_dport = this->client_port;

		tcp_header->th_seq = htonl(this->vars.snd.nxt);
		tcp_header->th_ack = htonl(this->vars.rcv.nxt);
		tcp_header->th_flags = (TH_SYN | TH_ACK);

		tcp_header->doff = 8;
		memcpy((void*)(tcp_header + 1), (void*)hs_opts, 12);
		server_packet.set_size(server_packet.get_size() + 12);

		this->send_tun(server_packet);

		do {
			*this > client_packet;
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);
		} while ((ntohl(tcp_header->th_seq) == this->vars.rcv.irs) && (tcp_header->th_flags == TH_SYN));

		if ((tcp_header->th_flags != TH_ACK) && (tcp_header->th_flags != TH_RST))
		{
			this->set_state(TCPSTATE_CLOSED);
			this->reset(client_packet);
			return -1;
		}

		this->vars.snd.nxt++;
		this->vars.snd.una = this->vars.snd.nxt;

		this->set_state(TCPSTATE_ESTABLISHED);
		// stk->set_nonblocking(true);

		return 0;
	}

	size_t TCPSocket::send_tun(Packet& packet)
	{
		utils::finalize_packet_tcp(&packet);
		return SessionSocket::send_tun(packet);
	}

	size_t TCPSocket::send(const Packet& packet)
	{
		ip* ip_header;
		tcphdr* tcp_header;
		utils::point_headers_tcp((Packet*)&packet, &ip_header, &tcp_header);

		if (tcp_header->th_flags == TH_SYN)
		{
			if (this->syn_recved)
			{
				return packet.get_size();
			}
			this->syn_recved = true;
		}

		return SessionSocket::send(packet);
	}

	size_t TCPSocket::send(const Buffer& buffer)
	{
		Packet packet;
		packet.set_protocol(TUNMODE_PROTOCOL_TCP);
		ip* ip_header;
		tcphdr* tcp_header;

		utils::build_tcp_packet(&packet);
		utils::point_headers_tcp(&packet, &ip_header, &tcp_header);

		ip_header->ip_src = this->server_addr;
		tcp_header->th_sport = this->server_port;
		ip_header->ip_dst = this->client_addr;
		tcp_header->th_dport = this->client_port;

		tcp_header->th_seq = htonl(this->vars.snd.nxt);
		tcp_header->th_ack = htonl(this->vars.rcv.nxt);
		tcp_header->th_flags = (TH_PUSH | TH_ACK);

		InBuffer in_buffer = packet.get_data();
		memcpy(in_buffer.get_buffer(), buffer.get_buffer(), buffer.get_size());
		packet.set_size(packet.get_size() + buffer.get_size());

		this->vars.snd.nxt += buffer.get_size();

		return this->send_tun(packet);
	}

	size_t TCPSocket::recv(Buffer& buffer)
	{
		Packet packet;
		packet.set_protocol(TUNMODE_PROTOCOL_TCP);
		ip* ip_header;
		tcphdr* tcp_header;

		*this > packet;
		InBuffer in_buffer = packet.get_data();

		utils::point_headers_tcp(&packet, &ip_header, &tcp_header);

		if (in_buffer.get_size() == 0)
		{
			if (!((this->vars.rcv.nxt <= ntohl(tcp_header->th_seq))
				&& (ntohl(tcp_header->th_seq) < (this->vars.rcv.nxt + this->vars.rcv.wnd))))
			{
				buffer.set_size(0);
				return 0;
			}
		}
		else
		{
			if ((!((this->vars.rcv.nxt <= ntohl(tcp_header->th_seq)) && (ntohl(tcp_header->th_seq) < (this->vars.rcv.nxt + this->vars.rcv.wnd))))
				|| !((this->vars.rcv.nxt <= (ntohl(tcp_header->th_seq) + in_buffer.get_size() - 1)) && ((ntohl(tcp_header->th_seq) + in_buffer.get_size() - 1) < (this->vars.rcv.nxt + this->vars.rcv.wnd))))
			{
				buffer.set_size(0);
				return 0;
			}
		}

		this->vars.rcv.nxt += in_buffer.get_size();

		switch (this->get_state())
		{
		case TCPSTATE_ESTABLISHED:
			{
				if (in_buffer.get_size() > 0)
				{
					if ((this->vars.snd.una < ntohl(tcp_header->th_ack))
						&& (ntohl(tcp_header->th_ack) <= this->vars.snd.nxt))
					{
						this->vars.snd.una = ntohl(tcp_header->th_ack);
					}

					buffer(in_buffer.get_buffer(), in_buffer.get_size());

					if (tcp_header->th_flags != TH_ACK)
					{
						Packet client_packet;
						client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
						ip* ip_header2;
						tcphdr* tcp_header2;
						utils::build_tcp_packet(&client_packet);
						utils::point_headers_tcp(&client_packet, &ip_header2, &tcp_header2);

						ip_header2->ip_src = this->server_addr;
						tcp_header2->th_sport = this->server_port;
						ip_header2->ip_dst = this->client_addr;
						tcp_header2->th_dport = this->client_port;

						tcp_header2->th_seq = htonl(this->vars.snd.nxt);
						tcp_header2->th_ack = htonl(this->vars.rcv.nxt);
						tcp_header2->th_flags = TH_ACK;

						return this->send_tun(client_packet);
					}

					return -2;    // MSG_MORE;
				}
				else if (tcp_header->th_flags == (TH_ACK))
				{
					if ((this->vars.snd.una < ntohl(tcp_header->th_ack))
						&& (ntohl(tcp_header->th_ack) <= this->vars.snd.nxt))
					{
						this->vars.snd.una = ntohl(tcp_header->th_ack);
					}

					buffer.set_size(0);
					return 0;
				}
				else if (tcp_header->th_flags & TH_FIN)
				{
					if ((this->vars.snd.una < ntohl(tcp_header->th_ack))
						&& (ntohl(tcp_header->th_ack) <= this->vars.snd.nxt))
					{
						this->vars.snd.una = ntohl(tcp_header->th_ack);
					}

					Packet client_packet;
					client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
					ip* ip_header2;
					tcphdr* tcp_header2;
					utils::build_tcp_packet(&client_packet);
					utils::point_headers_tcp(&client_packet, &ip_header2, &tcp_header2);

					this->vars.rcv.nxt += 1;

					ip_header2->ip_src = this->server_addr;
					tcp_header2->th_sport = this->server_port;
					ip_header2->ip_dst = this->client_addr;
					tcp_header2->th_dport = this->client_port;

					tcp_header2->th_seq = tcp_header->th_ack;
					tcp_header2->th_ack = htonl(ntohl(tcp_header->th_seq) + 0x1);
					tcp_header2->th_flags = TH_ACK;

					this->set_state(TCPSTATE_CLOSE_WAIT);
					this->send_tun(client_packet);
					buffer.set_size(0);
					return 0;
				}
				else
				{
					utils::print_packet_tcp(&packet);
					LOGW_("^ Unknown TCP flags during ESTABLISHED state");

					if (!(tcp_header->th_flags & TH_RST))
					{
						LOGW_("^ RST Sent");
						this->reset(packet);
					}

					buffer.set_size(0);
					return -1;
				}
			}
			break;

		case TCPSTATE_FIN_WAIT_1:
			break;
			
		case TCPSTATE_FIN_WAIT_2:
			break;
			
		case TCPSTATE_CLOSING:
			break;
			
		case TCPSTATE_TIME_WAIT:
			{
				if (tcp_header->th_flags == (TH_FIN | TH_ACK))
				{
					Packet client_packet;
					client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
					ip* ip_header2;
					tcphdr* tcp_header2;

					utils::build_tcp_packet(&client_packet);
					utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

					ip_header2->ip_src = this->server_addr;
					tcp_header2->th_sport = this->server_port;
					ip_header2->ip_dst = this->client_addr;
					tcp_header2->th_dport = this->client_port;

					tcp_header2->th_seq = htonl(this->vars.snd.nxt);
					tcp_header2->th_ack = htonl(this->vars.rcv.nxt);
					tcp_header2->th_flags = TH_ACK;

					this->send_tun(client_packet);
					buffer.set_size(0);
					return 0;
				}
				else
				{
					utils::print_packet_tcp(&packet);
					LOGW_("^ Unknown TCP flags during TIME_WAIT state");
					LOGW_("^ RST Sent");
					this->reset(packet);
				}
			}

			break;
			
		case TCPSTATE_CLOSE_WAIT:
			{
				if (tcp_header->th_flags == TH_ACK)
				{
					buffer.set_size(0);
					return 0;
				}
				else
				{
					utils::print_packet_tcp(&packet);
					LOGW_("^ Unknown TCP flags during CLOSE_WAIT state");
					LOGW_("^ RST Sent");
					this->reset(packet);
				}
			}

			break;
			
		case TCPSTATE_LAST_ACK:
			{
				if ((tcp_header->th_flags == TH_ACK) && (tcp_header->th_ack == htonl(this->vars.snd.una)))
				{
					buffer.set_size(0);
					return 0;
				}
				else
				{
					utils::print_packet_tcp(&packet);
					LOGW_("^ Unknown TCP flags during LAST_ACK state");
					LOGW_("^ RST Sent");
					this->reset(packet);
				}
			}

			break;
			
		case TCPSTATE_CLOSED:
			{
				this->reset(packet);
			}

			break;
		
		default:
			break;
		}

		return -1;
	}

	void TCPSocket::operator<<(const Buffer& buffer)
	{

	}

	void TCPSocket::operator>>(Buffer& buffer)
	{

	}

	void TCPSocket::close()
	{
		TCPState state = this->get_state();
		if (state == TCPSTATE_ESTABLISHED)
		{
			Packet client_packet;
			client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
			ip* ip_header;
			tcphdr* tcp_header;

			utils::build_tcp_packet(&client_packet);
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			ip_header->ip_src = this->server_addr;
			tcp_header->th_sport = this->server_port;
			ip_header->ip_dst = this->client_addr;
			tcp_header->th_dport = this->client_port;

			tcp_header->th_seq = htonl(this->vars.snd.nxt);
			tcp_header->th_ack = htonl(this->vars.rcv.nxt);
			tcp_header->th_flags = TH_FIN | TH_ACK;

			this->send_tun(client_packet);
			this->set_state(TCPSTATE_FIN_WAIT_1);

			*this > client_packet; // assume packet is ACK of FIN

			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			if (tcp_header->th_flags == TH_ACK)
			{
				this->set_state(TCPSTATE_FIN_WAIT_2);
				*this > client_packet; // assume packet is FIN | ACK
			}

			if (ntohl(tcp_header->th_seq) > this->vars.rcv.nxt)
			{
				this->vars.rcv.nxt = ntohl(tcp_header->th_seq) + 1;
				this->vars.snd.nxt = ntohl(tcp_header->th_ack);
			}
			else
			{
				this->vars.rcv.nxt += 1;
				this->vars.snd.nxt += 1;
			}

			utils::build_tcp_packet(&client_packet);

			ip_header->ip_src = this->server_addr;
			tcp_header->th_sport = this->server_port;
			ip_header->ip_dst = this->client_addr;
			tcp_header->th_dport = this->client_port;

			tcp_header->th_seq = htonl(this->vars.snd.nxt);
			tcp_header->th_ack = htonl(this->vars.rcv.nxt);
			tcp_header->th_flags = TH_ACK;

			LOGD_("ESTABLISHED | Closing connection");

			this->set_state(TCPSTATE_TIME_WAIT);
			this->send_tun(client_packet);
		}
		else if (state == TCPSTATE_CLOSE_WAIT)
		{
			Packet client_packet;
			client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
			ip* ip_header;
			tcphdr* tcp_header;

			utils::build_tcp_packet(&client_packet);
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			ip_header->ip_src = this->server_addr;
			tcp_header->th_sport = this->server_port;
			ip_header->ip_dst = this->client_addr;
			tcp_header->th_dport = this->client_port;

			tcp_header->th_seq = htonl(this->vars.snd.nxt);
			tcp_header->th_ack = htonl(this->vars.rcv.nxt);
			tcp_header->th_flags = TH_FIN | TH_ACK;

			this->send_tun(client_packet);
			this->set_state(TCPSTATE_LAST_ACK);

			*this > client_packet; // assume packet is ACK of FIN

			LOGD_("CLOSE_WAIT | Closing connection");

			this->set_state(TCPSTATE_CLOSED);
		}
	}

	TCPState TCPSocket::get_state()
	{
		return this->state;
	}

	void TCPSocket::set_state(TCPState state)
	{
		this->state = state;
	}

	void TCPSocket::reset(const Packet& packet)
	{
		ip* ip_header;
		tcphdr* tcp_header;
		utils::point_headers_tcp(&packet, &ip_header, &tcp_header);

		in_addr client_addr = ip_header->ip_src;
		in_addr server_addr = ip_header->ip_dst;
		u_short client_port = tcp_header->th_sport;
		u_short server_port = tcp_header->th_dport;

		uint32_t pkt_seq = tcp_header->th_seq;
		uint32_t pkt_ack = tcp_header->th_ack;
		uint8_t  pkt_flags = tcp_header->th_flags;

		InBuffer in_buffer = packet.get_data();
		size_t   data_size = in_buffer.get_size();

		TCPState state = this->get_state();
		if (state == TCPSTATE_CLOSED)
		{
			if (pkt_flags & TH_RST)
			{
				return;
			}

			Packet client_packet;
			client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
			utils::build_tcp_packet(&client_packet);
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			ip_header->ip_src = server_addr;
			tcp_header->th_sport = server_port;
			ip_header->ip_dst = client_addr;
			tcp_header->th_dport = client_port;

			tcp_header->th_flags = TH_RST;

			if (pkt_ack)
			{
				tcp_header->th_seq = pkt_ack;
				tcp_header->th_ack = 0;
			}
			else
			{
				tcp_header->th_seq = 0;
				tcp_header->th_ack = htonl(ntohl(pkt_seq) + data_size);
			}

			this->send_tun(client_packet);
		}
		else if ((state == TCPSTATE_SYN_SENT)
				|| (state == TCPSTATE_SYN_RECEIVED)
				|| (state == TCPSTATE_LISTEN))
		{
			auto tcp_flags = tcp_header->th_flags;

			Packet client_packet;
			client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
			utils::build_tcp_packet(&client_packet);
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			ip_header->ip_src = server_addr;
			tcp_header->th_sport = server_port;
			ip_header->ip_dst = client_addr;
			tcp_header->th_dport = client_port;

			tcp_header->th_flags = TH_RST;

			if (pkt_ack)
			{
				tcp_header->th_seq = pkt_ack;
				tcp_header->th_ack = 0;
			}
			else
			{
				tcp_header->th_seq = 0;
				tcp_header->th_ack = htonl(ntohl(pkt_seq) + data_size + (tcp_flags == TH_SYN ? 1 : 0));
			}

			this->send_tun(client_packet);
		}
		else
		{
			Packet client_packet;
			client_packet.set_protocol(TUNMODE_PROTOCOL_TCP);
			utils::build_tcp_packet(&client_packet);
			utils::point_headers_tcp(&client_packet, &ip_header, &tcp_header);

			ip_header->ip_src = server_addr;
			tcp_header->th_sport = server_port;
			ip_header->ip_dst = client_addr;
			tcp_header->th_dport = client_port;

			tcp_header->th_flags = TH_RST;

			tcp_header->th_seq = pkt_ack;
			tcp_header->th_ack = 0;

			this->send_tun(client_packet);
		}

		this->set_state(TCPSTATE_CLOSED);
	}
}