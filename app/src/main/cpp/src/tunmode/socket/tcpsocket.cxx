#include <tunmode/socket/tcpsocket.hpp>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>

namespace tunmode
{
	TCPSocket::TCPSocket() : SessionSocket()
	{
		this->state = LISTEN;
	}

	TCPSocket::~TCPSocket()
	{
		SessionSocket::~SessionSocket();
	}

	Socket* TCPSocket::connect()
	{
		return nullptr;
	}

	size_t TCPSocket::send(const Buffer& buffer)
	{
		return 0;
	}

	size_t TCPSocket::recv(Buffer& buffer)
	{
		return 0;
	}

	void TCPSocket::operator<<(const Buffer& buffer)
	{

	}

	void TCPSocket::operator>>(Buffer& buffer)
	{

	}

	TCPState TCPSocket::get_state()
	{
		return this->state;
	}
}