#pragma once
#include <span>
#include <glad/gl.h>

class Buffer
{
public:
    Buffer(GLsizeiptr size) noexcept
        : Buffer(nullptr, size) { }
        
    Buffer(const std::span<std::byte> data) noexcept
        : Buffer(data.data(), (GLsizeiptr)data.size_bytes()) { }

    Buffer(const void *data, GLsizeiptr size) noexcept;

    ~Buffer() noexcept;

    void Write(const std::span<std::byte> data, GLintptr offset = 0);
    void Write(const void *data, GLsizeiptr dataSize, GLintptr offset = 0);

    template <typename T>
    void Write(const T& data) { Write(&data, sizeof(T)); }

    constexpr GLuint GetID() const noexcept { return id_; }
private:
    GLuint id_;
    GLsizeiptr size_;
};