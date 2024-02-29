#include <tunmode/socket/socket.hpp>

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace tunmode
{
	Socket::Socket(int domain, int type, int protocol) : closed{false}
	{
		this->socket = ::socket(domain, type, protocol);
	}

	Socket::Socket(int socket) : closed{false}
	{
		this->socket = socket;
	}

	Socket::~Socket()
	{
		if (!this->closed)
			::close(this->socket);
	}

	int Socket::bind(in_addr addr, u_short port)
	{
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr = addr;
		address.sin_port = port;

		return ::bind(this->socket, (struct sockaddr*)&address, sizeof(address));
	}

	int Socket::connect(in_addr addr, u_short port)
	{
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr = addr;
		address.sin_port = port;

		return ::connect(this->socket, (struct sockaddr*)&address, sizeof(address));
	}

	int Socket::close()
	{
		this->closed = true;
		return ::close(this->socket);
	}

	size_t Socket::send(const Buffer* buffer, int flags)
	{
		return ::send(this->socket, buffer->get_buffer(), buffer->get_size(), flags);
	}

	size_t Socket::recv(Buffer* buffer, int flags)
	{
		size_t size = ::recv(this->socket, buffer->get_buffer(), buffer->get_size(), flags);

		if (size != -1)
		{
			buffer->set_size(size);
		}

		return size;
	}

	int Socket::get_socket()
	{
		return this->socket;
	}

	int Socket::set_nonblocking(bool state)
	{
		int flags = fcntl(this->socket, F_GETFL, 0);
		if (flags == -1) return -1;
		flags = state ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		return fcntl(this->socket, F_SETFL, flags);
	}

	void Socket::operator<<(const Buffer& buffer)
	{
		this->send(&buffer);
	}

	void Socket::operator<<(const Buffer* buffer)
	{
		this->send(buffer);
	}

	void Socket::operator>>(Buffer& buffer)
	{
		this->recv(&buffer);
	}

	void Socket::operator>>(Buffer* buffer)
	{
		this->recv(buffer);
	}
}