#include <tunmode/manager/tcpmanager.hpp>
#include <tunmode/session/tcpsession.hpp>

namespace tunmode
{
	TCPManager::TCPManager() : SessionManager() {}

	void TCPManager::handle_packet(const Packet& packet)
	{
		TCPSession* session = reinterpret_cast<TCPSession*>(this->get_or_add(packet.get_id()));
		*(session->get_client_socket()) << packet;
	}

	Session* TCPManager::add(uint64_t id)
	{
		TCPSession* session = new TCPSession(id);
		this->sessions.insert({id, session});

		return session;
	}
}