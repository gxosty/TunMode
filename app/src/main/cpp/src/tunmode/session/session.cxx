#include <tunmode/session/session.hpp>

#include <thread>

namespace tunmode
{
	in_addr Session::iface_addr = in_addr{0};

	Session::Session(uint64_t id)
	{
		this->id = id;

		std::thread loop_t(&Session::loop, this);
		loop_t.detach();
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
}