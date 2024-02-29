#include <tunmode/session/tcpsession.hpp>

#include <tunmode/socket/sessionsocket.hpp>
#include <tunmode/socket/tcpsocket.hpp>

namespace tunmode
{
	TCPSession::TCPSession(uint64_t id) : Session(id)
	{
		this->client_socket = reinterpret_cast<SessionSocket*>(new TCPSocket());
	}

	TCPSession::~TCPSession()
	{
		Session::~Session();
	}

	int TCPSession::poll()
	{
		return 0;
	}

	void TCPSession::loop()
	{

	}
}