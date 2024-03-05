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
		Buffer(const void* buffer, const size_t& size);

		void operator()(const void* buffer, const size_t& size);

	private:
		char _buffer[TUNMODE_BUFFER_SIZE + 200];

		void _copy(const void* buffer, const size_t& size);
	};
}