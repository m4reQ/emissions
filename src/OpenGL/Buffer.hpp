#pragma once
#include <span>
#include <glad/gl.h>

class Buffer
{
public:
    Buffer(GLsizeiptr size) noexcept;
    Buffer(const std::span<std::byte> data);
    Buffer(const void *data, GLsizeiptr size) noexcept;

    ~Buffer() noexcept;

    void Write(const std::span<std::byte> data, GLintptr offset = 0);
    void Write(const void *data, GLsizeiptr dataSize, GLintptr offset = 0);

    template <typename T>
    void Write(const T& data) { Write(&data, sizeof(T)); }

    constexpr GLuint GetID() const noexcept { return id_; }
private:
    GLuint id_;
};