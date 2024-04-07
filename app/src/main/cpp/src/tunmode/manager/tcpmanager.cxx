#include <tunmode/manager/tcpmanager.hpp>
#include <tunmode/session/tcpsession.hpp>
#include <tunmode/common/utils.hpp>

namespace tunmode
{
	TCPManager::TCPManager() : SessionManager() {}

	void TCPManager::handle_packet(const Packet& packet)
	{
		std::lock_guard<std::mutex> lock(this->mtx);
		TCPSession* session = reinterpret_cast<TCPSession*>(this->get_or_add(packet.get_id()));
		*(session->get_client_socket()) < packet;
	}

	Session* TCPManager::add(uint64_t id)
	{
		TCPSession* session = new TCPSession(this, id);
		utils::protect_socket(session->get_server_socket()->get_socket());
		this->sessions.insert({id, session});

		return session;
	}
}