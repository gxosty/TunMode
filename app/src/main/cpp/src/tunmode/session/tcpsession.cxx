#include <tunmode/session/tcpsession.hpp>

#include <tunmode/tunmode.hpp>
#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/socket/tcpsocket.hpp>
#include <tunmode/manager/tcpmanager.hpp>

#include <sys/socket.h>
#include <thread>
#include <errno.h>

namespace tunmode
{
	TCPSession::TCPSession(TCPManager* manager, uint64_t id) : Session(id)
	{
		this->manager = manager;
		this->client_socket = reinterpret_cast<SessionSocket*>(new TCPSocket());
		this->server_socket = new Socket(AF_INET, SOCK_STREAM);
		// this->server_socket->set_nonblocking(true);

		std::thread loop_t(&TCPSession::loop, this);
		loop_t.detach();
	}

	int TCPSession::poll(struct pollfd fds[2])
	{
		return 0;
	}

	int TCPSession::poll2(struct pollfd fds[2], Buffer& client_buffer, Buffer& server_buffer)
	{
		TCPSocket* cl_socket = reinterpret_cast<TCPSocket*>(this->client_socket);
		Socket*& sv_socket = this->server_socket;

		TCPState cl_socket_state = cl_socket->get_state();

		int ret = 0;
		int _ret = 0;

		while (true)
		{
			_ret = 0;

			// if (cl_socket_state == TCPSTATE_CLOSED)
			// {
			// 	fds[0].revents = POLLNVAL;
			// 	return 1;
			// }

			int fd_count = 2;
			int poll_timeout = 120000;

			if ((cl_socket_state == TCPSTATE_TIME_WAIT) || (cl_socket_state == TCPSTATE_CLOSED))
			{
				fd_count = 1;
				poll_timeout = 15000;
				fds[1].revents = 0;
			}

			ret = ::poll(fds, fd_count, poll_timeout);

			if (ret == -1)
			{
				break;
			}
			else if (ret == 0)
			{
				if ((cl_socket_state == TCPSTATE_TIME_WAIT) || (cl_socket_state == TCPSTATE_CLOSED))
				{
					fds[0].revents = POLLNVAL;
					return 1;
				}

				return 0;
			}
			else
			{
				if (fds[0].revents)
				{
					_ret++;
					if (fds[0].revents & POLLIN)
					{
						int status = cl_socket->recv(client_buffer);
						if (status == -2)
						{
							fds[0].revents |= POLLMSG;
						}
						else if (status == -1)
						{
							if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
							{
								fds[0].revents = 0;
								ret--;
							}
							else
							{
								fds[0].revents = POLLERR;
							}
						}
						else if (client_buffer.get_size() == 0)
						{
							if (cl_socket->get_state() == TCPSTATE_CLOSE_WAIT)
							{
								fds[0].revents = POLLHUP;
							}
							else
							{
								fds[0].revents = 0;
								_ret--;
							}
						}
					}
				}

				if (fds[1].revents)
				{
					_ret++;

					if (fds[1].revents & POLLIN)
					{
						size_t sz = sv_socket->recv(&server_buffer, MSG_DONTWAIT);

						if (sz == 0)
						{
							fds[1].revents = POLLHUP;
						}
						else if (sz == -1)
						{
							if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
							{
								fds[1].revents = 0;
								ret--;
							}
						}
					}
				}

				if (_ret)
				{
					return _ret;
				}
			}
		}

		return -1;
	}

	void TCPSession::loop()
	{
		this->_loop();
		this->manager->remove(this->id);
	}

	void TCPSession::_loop()
	{
		TCPSocket* cl_socket = reinterpret_cast<TCPSocket*>(this->client_socket);
		Socket*& sv_socket = this->server_socket;

		if (cl_socket->connect(sv_socket))
		{
			cl_socket->close();
			sv_socket->close();
			return;
		}

		struct pollfd fds[2];

		fds[0].fd = cl_socket->get_read_pipe();
		fds[0].events = POLLIN; // fake POLLIN

		fds[1].fd = sv_socket->get_socket();
		fds[1].events = POLLIN;

		Buffer client_buffer;
		Buffer server_buffer;

		while (!tunmode::params::stop_flag.load())
		{
			fds[0].revents = 0;
			fds[1].revents = 0;
			int ret = this->poll2(fds, client_buffer, server_buffer);

			if (ret == -1)
			{
				break;
			}
			else if (ret == 0)
			{
				// timeout reached
			}
			else
			{
				if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					break;
				}
				else if (fds[0].revents & POLLIN)
				{
					sv_socket->send(&client_buffer, fds[0].revents & POLLMSG ? MSG_MORE : 0);
				}

				if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					break;
				}
				else if (fds[1].revents & POLLIN)
				{
					cl_socket->send(server_buffer);
				}
			}
		}

		cl_socket->close();
		sv_socket->close();
	}
}