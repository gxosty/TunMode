#include <tunmode/session/session.hpp>

#include <thread>

#include <misc/logger.hpp>

namespace tunmode
{
	Session::Session(uint64_t id)
	{
		LOGD_("Session::Session");
		this->id = id;
	}

	Session::~Session()
	{
		delete this->client_socket;
		delete this->server_socket;
		LOGD_("Session::~Session");
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