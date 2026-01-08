#include "Buffer.hpp"
#include <limits>
#include <stdexcept>

Buffer::Buffer(const void *data, GLsizeiptr size) noexcept
    : size_(size)
{
    glCreateBuffers(1, &id_);
    glNamedBufferStorage(id_, size, nullptr, data ? 0 : GL_DYNAMIC_STORAGE_BIT);
}

Buffer::Buffer(Buffer &&other) noexcept
{
    id_ = std::exchange(other.id_, 0);
}

Buffer::~Buffer() noexcept
{
    glDeleteBuffers(1, &id_);
}

Buffer &Buffer::operator=(Buffer &&other) noexcept
{
    id_ = std::exchange(other.id_, 0);

    return *this;
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
