#include <tunmode/manager/udpmanager.hpp>
#include <tunmode/session/udpsession.hpp>
#include <tunmode/common/utils.hpp>

namespace tunmode
{
	UDPManager::UDPManager() : SessionManager() {}

	void UDPManager::handle_packet(const Packet& packet)
	{
		std::lock_guard<std::mutex> lock(this->mtx);
		UDPSession* session = reinterpret_cast<UDPSession*>(this->get_or_add(packet.get_id()));
		*(session->get_client_socket()) < packet;
	}

	Session* UDPManager::add(uint64_t id)
	{
		UDPSession* session = new UDPSession(this, id);
		utils::protect_socket(session->get_server_socket()->get_socket());
		this->sessions.insert({id, session});

		return session;
	}
}