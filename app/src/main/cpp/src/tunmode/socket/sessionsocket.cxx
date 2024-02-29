#include <tunmode/socket/sessionsocket.hpp>

#include <unistd.h>

namespace tunmode
{
	TunSocket* SessionSocket::tun = nullptr;

	SessionSocket::SessionSocket()
	{
		this->session_pipe[0] = 0;
		this->session_pipe[1] = 0;

		pipe2(this->session_pipe, O_DIRECT);
	}

	SessionSocket::~SessionSocket()
	{
		::close(this->session_pipe[0]);
		::close(this->session_pipe[1]);
	}

	size_t SessionSocket::send(const Packet& packet)
	{
		return ::write(this->session_pipe[1], packet.get_buffer(), packet.get_size());
	}

	size_t SessionSocket::recv(Packet& packet)
	{
		size_t size = ::read(this->session_pipe[0], packet.get_buffer(), packet.get_size());
		packet.set_size(size);
		return size;
	}

	void SessionSocket::operator<<(const Packet& packet)
	{
		this->send(packet);
	}

	void SessionSocket::operator>>(Packet& packet)
	{
		this->recv(packet);
	}
}