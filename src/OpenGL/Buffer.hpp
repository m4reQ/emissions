#pragma once
#include <span>
#include <glad/gl.h>

class Buffer
{
public:
    Buffer() = default;
    Buffer(GLsizeiptr size) noexcept
        : Buffer(nullptr, size) { }
    Buffer(const std::span<std::byte> data) noexcept
        : Buffer(data.data(), (GLsizeiptr)data.size_bytes()) { }
    Buffer(const void *data, GLsizeiptr size) noexcept;
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;

    ~Buffer() noexcept;

    Buffer& operator=(Buffer&& other) noexcept;

    void Write(const std::span<std::byte> data, GLintptr offset = 0);
    void Write(const void *data, GLsizeiptr dataSize, GLintptr offset = 0);

    constexpr GLuint GetID() const noexcept { return id_; }
    constexpr GLsizeiptr GetSize() const noexcept { return size_; }
private:
    GLuint id_;
    GLsizeiptr size_;
};