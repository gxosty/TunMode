#include <tunmode/common/packet.hpp>

namespace tunmode
{
	Packet::Packet() : Buffer()
	{
		this->id = 0;
		this->protocol = 0;
	}

	uint64_t Packet::get_id() const
	{
		return this->id;
	}

	int Packet::get_protocol() const
	{
		return this->protocol;
	}

	InBuffer Packet::get_data() const
	{
		return InBuffer(this->buffer, this->size);
	}

	void Packet::set_id(uint64_t id)
	{
		this->id = id;
	}

	void Packet::set_protocol(int protocol)
	{
		this->protocol = protocol;
	}
}