#include <tunmode/common/inbuffer.hpp>

namespace tunmode
{
	InBuffer::InBuffer()
	{
		this->size = 0;
	}

	InBuffer::InBuffer(void* buffer, size_t size)
	{
		this->buffer = buffer;
		this->size = size;
	}

	void* InBuffer::get_buffer() const
	{
		return this->buffer;
	}

	size_t InBuffer::get_size() const
	{
		return this->size;
	}

	void InBuffer::set_size(size_t size)
	{
		this->size = size;
	}
}