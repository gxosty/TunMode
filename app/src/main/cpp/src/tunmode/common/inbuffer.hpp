#pragma once

#include <cstdint>

namespace tunmode
{
	class InBuffer
	{
	public:
		InBuffer();
		InBuffer(void* buffer, size_t size);

		void*  get_buffer() const;
		size_t get_size() const;

		void   set_size(size_t size);

	protected:
		void* buffer;
		size_t size;
	};
}