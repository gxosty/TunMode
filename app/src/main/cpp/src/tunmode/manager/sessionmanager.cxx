#include <tunmode/manager/sessionmanager.hpp>

namespace tunmode
{
	SessionManager::SessionManager() {}

	Session* SessionManager::get_or_add(uint64_t id)
	{
		if (this->sessions.contains(id))
		{
			return this->sessions[id];
		}

		return this->add(id);
	}

	void SessionManager::remove(uint64_t id)
	{
		std::lock_guard<std::mutex> lock(this->mtx);
		
		Session* session = this->sessions[id];
		this->sessions.erase(id);
		delete session;
	}
}