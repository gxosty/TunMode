#pragma once

#include <cstdint>

#include "buffer.hpp"

namespace tunmode
{
	class Packet : public Buffer
	{
	public:
		Packet();

		uint64_t get_id() const;
		int      get_protocol() const;
		InBuffer get_data() const;

		void     set_id(uint64_t id);
		void     set_protocol(int protocol);

	private:
		uint64_t id;
		int protocol;
	};
}