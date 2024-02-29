#pragma once

#include "inbuffer.hpp"
#include "../definitions.hpp"

#include <cstdint>

namespace tunmode
{
	class Buffer : public InBuffer
	{
	public:
		Buffer();
		Buffer(void* buffer, size_t size);

	private:
		char _buffer[TUNMODE_BUFFER_SIZE];
	};
}