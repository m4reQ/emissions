#pragma once
#include <span>
#include <glad/gl.h>

class Buffer
{
public:
    Buffer(GLsizeiptr size) noexcept;
    Buffer(const std::span<std::byte> data);
    Buffer(const void *data, GLsizeiptr size) noexcept;

    template <typename T>
    Buffer(const T& value) noexcept
        : Buffer(&value, sizeof(T)) { }

    template <typename T>
    Buffer(size_t elementsCount) noexcept
        : Buffer(sizeof(T) * elementsCount) { }

    ~Buffer() noexcept;

    void Write(const std::span<std::byte> data, GLintptr offset = 0);
    void Write(const void *data, GLsizeiptr dataSize, GLintptr offset = 0);

    constexpr GLuint GetID() const noexcept { return id_; }
private:
    GLuint id_;
};