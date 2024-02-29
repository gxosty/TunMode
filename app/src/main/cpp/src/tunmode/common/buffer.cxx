#include <tunmode/common/buffer.hpp>

#include <cstdlib>
#include <cstring>

namespace tunmode
{
	Buffer::Buffer() : InBuffer()
	{
		this->buffer = (void*)&(this->_buffer[0]);
	}

	Buffer::Buffer(void* buffer, size_t size)
	{
		size = fmin(size, TUNMODE_BUFFER_SIZE);
		memcpy(this->_buffer, buffer, size);
		this->buffer = (void*)&(this->_buffer[0]);
		this->size = size;
	}
}