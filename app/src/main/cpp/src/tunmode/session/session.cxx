#include <tunmode/session/session.hpp>

#include <thread>

namespace tunmode
{
	Session::Session(uint64_t id)
	{
		this->id = id;
	}

	Session::~Session()
	{
		delete this->client_socket;
		delete this->server_socket;
	}

	uint64_t Session::get_id()
	{
		return this->id;
	}

	SessionSocket* Session::get_client_socket()
	{
		return this->client_socket;
	}

	Socket* Session::get_server_socket()
	{
		return this->server_socket;
	}
}