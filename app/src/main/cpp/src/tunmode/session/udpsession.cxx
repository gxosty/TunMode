#include <tunmode/session/udpsession.hpp>

#include <tunmode/tunmode.hpp>
#include <tunmode/socket/socket.hpp>
#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/socket/udpsocket.hpp>
#include <tunmode/manager/udpmanager.hpp>

#include <sys/socket.h>
#include <thread>
#include <errno.h>

#include <misc/logger.hpp>

namespace tunmode
{
	UDPSession::UDPSession(UDPManager* manager, uint64_t id) : Session(id)
	{
		this->manager = manager;
		this->client_socket = reinterpret_cast<SessionSocket*>(new UDPSocket());
		this->server_socket = new Socket(AF_INET, SOCK_DGRAM);
		this->poll_timeout = 60000;

		std::thread loop_t(&UDPSession::loop, this);
		loop_t.detach();
	}

	int UDPSession::poll(struct pollfd fds[2])
	{
		return ::poll(fds, 2, 60000);
	}

	void UDPSession::loop()
	{
		this->_loop();
		this->manager->remove(this->id);
	}

	void UDPSession::_loop()
	{
		UDPSocket* cl_socket = reinterpret_cast<UDPSocket*>(this->client_socket);
		Socket*& sv_socket = this->server_socket;

		cl_socket->init(sv_socket);

		struct pollfd fds[2];

		fds[0].fd = cl_socket->get_read_pipe();
		fds[0].events = POLLIN; // fake POLLIN

		fds[1].fd = sv_socket->get_socket();
		fds[1].events = POLLIN;

		Buffer client_buffer;
		Buffer server_buffer;

		while (!tunmode::params::stop_flag.load())
		{
			int ret = this->poll(fds);

			if (ret == -1)
			{
				break;
			}
			else if (ret == 0)
			{
				// timeout reached
				// server didn't respond
				break;
			}
			else
			{
				if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					break;
				}
				else if (fds[0].revents & POLLIN)
				{
					*cl_socket >> client_buffer;
					*sv_socket << client_buffer;
				}

				if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					break;
				}
				else if (fds[1].revents & POLLIN)
				{
					*sv_socket >> server_buffer;
					*cl_socket << server_buffer;
					this->poll_timeout = 10000;
				}
			}
		}

		cl_socket->close();
		sv_socket->close();
	}
}