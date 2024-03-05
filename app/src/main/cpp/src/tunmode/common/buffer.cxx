#include <tunmode/common/buffer.hpp>

#include <cstdlib>
#include <cstring>

namespace tunmode
{
	Buffer::Buffer() : InBuffer()
	{
		this->buffer = (void*)&(this->_buffer[0]);
	}

	Buffer::Buffer(const void* buffer, const size_t& size)
	{
		this->_copy(buffer, size);
	}

	void Buffer::operator()(const void* buffer, const size_t& size)
	{
		this->_copy(buffer, size);
	}

	void Buffer::_copy(const void* buffer, const size_t& size)
	{
		size_t _size = fmin(size, TUNMODE_BUFFER_SIZE);
		memcpy(this->_buffer, buffer, _size);
		this->buffer = (void*)&(this->_buffer[0]);
		this->size = _size;
	}
}