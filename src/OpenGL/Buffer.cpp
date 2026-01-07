#include "Buffer.hpp"
#include <limits>
#include <stdexcept>

Buffer::Buffer(GLsizeiptr size) noexcept
{
    glCreateBuffers(1, &id_);
    glNamedBufferStorage(id_, size, nullptr, 0);
}

Buffer::~Buffer() noexcept
{
    glDeleteBuffers(1, &id_);
}

void Buffer::Write(const std::span<std::byte> data, GLintptr offset)
{
    if (data.size_bytes() >= std::numeric_limits<GLsizeiptr>::max())
        throw std::overflow_error("Data size exceeds size accepted by OpenGL call.");

    Write(data.data(), data.size_bytes(), offset);
}

void Buffer::Write(const void *data, GLsizeiptr dataSize, GLintptr offset)
{
    glNamedBufferSubData(id_, offset, dataSize, data);
}
