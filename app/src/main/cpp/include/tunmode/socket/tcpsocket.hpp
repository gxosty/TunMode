#pragma once

#include "socket.hpp"
#include "sessionsocket.hpp"
#include "../common/packet.hpp"
#include "../common/buffer.hpp"

#include <netinet/in.h>
#include <cstdint>

namespace tunmode
{
	typedef struct __TCP_VARS__ {
		struct {
			uint32_t una{0};
			uint32_t nxt{0};
			u_short  wnd{0};
			u_short  up{0};
			uint32_t wl1{0};
			uint32_t wl2{0};
			uint32_t iss{0};
		} snd;

		struct {
			uint32_t nxt{0};
			uint32_t wnd{0};
			u_short  up{0};
			uint32_t irs{0};
		} rcv;
	} TCPVars;

	enum TCPState {
		TCPSTATE_LISTEN = 0,
		TCPSTATE_SYN_SENT,
		TCPSTATE_SYN_RECEIVED,
		TCPSTATE_ESTABLISHED,
		TCPSTATE_FIN_WAIT_1,
		TCPSTATE_FIN_WAIT_2,
		TCPSTATE_CLOSE_WAIT,
		TCPSTATE_CLOSING,
		TCPSTATE_LAST_ACK,
		TCPSTATE_TIME_WAIT,
		TCPSTATE_CLOSED
	};

	class TCPSocket : public SessionSocket
	{
	public:
		TCPSocket();
		~TCPSocket() override;

		int connect(Socket* skt);

		size_t send_tun(Packet& packet) override;
		size_t send(const Buffer& buffer) override;
		size_t recv(Buffer& buffer) override;

		void   operator<<(const Buffer& buffer) override;
		void   operator>>(Buffer& buffer) override;

		void close();

		TCPState get_state();

	private:
		in_addr client_addr;
		u_short client_port;
		in_addr server_addr;
		u_short server_port;

		TCPVars vars;
		TCPState state;

		void set_state(TCPState state);
		void reset(const Packet& packet);
	};
}