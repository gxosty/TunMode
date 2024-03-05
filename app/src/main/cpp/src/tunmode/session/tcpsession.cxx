#include <tunmode/session/tcpsession.hpp>

#include <tunmode/tunmode.hpp>
#include <tunmode/socket/socket.hpp>
#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/socket/tcpsocket.hpp>
#include <tunmode/manager/tcpmanager.hpp>

#include <sys/socket.h>
#include <thread>
#include <errno.h>

#include <misc/logger.hpp>

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

			if ((cl_socket_state == TCPSTATE_CLOSED) || (cl_socket_state == TCPSTATE_TIME_WAIT))
			{
				fds[0].revents = POLLNVAL;
				return 1;
			}

			ret = ::poll(fds, 2, 120000);

			if (ret == -1)
			{
				break;
			}
			else if (ret == 0)
			{
				return 0;
			}
			else
			{
				if (fds[0].revents)
				{
					_ret++;
					LOGD_("poll2: revents == %d", (int)fds[0].revents);
					if (fds[0].revents & POLLIN)
					{
						int status = cl_socket->recv(client_buffer);
						if (status == -2)
						{
							fds[0].revents |= POLLMSG;
						}
						else if (status == -1)
						{
							fds[0].revents = POLLERR;
						}
						else if (client_buffer.get_size() == 0)
						{
							fds[0].revents = 0;
							_ret--;
						}
					}
				}

				if (fds[1].revents)
				{
					if (fds[1].revents & POLLIN)
					{
						if (sv_socket->recv(&server_buffer) == 0)
						{
							fds[1].revents = POLLHUP;
						}

#ifndef RELEASE_MODE
						if (server_buffer.get_size() == 0)
						{
							LOGD_("server_buffer.get_size() == 0");
						}
#endif
					}

					_ret++;
				}

				if (_ret)
				{
					return _ret;
				}
			}
		}

		LOGE_("REACHED POLL2 END");

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

		LOGD_("Calling connect");
		cl_socket->connect(sv_socket);
		LOGD_("Connect end");

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
			LOGD_("_loop: revents == %d", (int)fds[0].revents);

			if (ret == -1)
			{
				LOGE_("Session poll error");
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
					LOGD_("cl_socket revents == %d", fds[0].revents);
					break;
				}
				else if (fds[0].revents & POLLIN)
				{
					LOGD_("-> server: %lu | %d", client_buffer.get_size(), (int)(fds[0].revents & POLLMSG ? MSG_MORE : 0));
					sv_socket->send(&client_buffer, fds[0].revents & POLLMSG ? MSG_MORE : 0);
				}

				if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					LOGD_("sv_socket revents == %d", fds[1].revents);
					if (fds[1].revents & POLLERR)
					{
						LOGE_("POLLERR: %s", std::strerror(errno));
					}
					if (fds[1].revents & POLLHUP)
					{
						LOGE_("POLLHUP");
					}
					if (fds[1].revents & POLLNVAL)
					{
						LOGE_("POLLNVAL");
					}

					break;
				}
				else if (fds[1].revents & POLLIN)
				{
					LOGD_("<- client: %lu", server_buffer.get_size());
					cl_socket->send(server_buffer);
				}
			}
		}

		cl_socket->close();
		sv_socket->close();
	}
}