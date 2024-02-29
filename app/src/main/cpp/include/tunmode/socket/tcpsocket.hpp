#pragma once

#include "socket.hpp"
#include "sessionsocket.hpp"
#include "../common/packet.hpp"
#include "../common/buffer.hpp"

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
		LISTEN = 0,
		SYN_SENT,
		SYN_RECEIVED,
		ESTABLISHED,
		FIN_WAIT_1,
		FIN_WAIT_2,
		CLOSE_WAIT,
		CLOSING,
		LAST_ACK,
		TIME_WAIT,
		CLOSED
	};

	class TCPSocket : public SessionSocket
	{
	public:
		TCPSocket();
		~TCPSocket() override;

		Socket* connect();

		size_t send(const Buffer& buffer) override;
		size_t recv(Buffer& buffer) override;

		void   operator<<(const Buffer& buffer) override;
		void   operator>>(Buffer& buffer) override;

		TCPState get_state();

	private:
		TCPVars vars;
		TCPState state;
	};
}