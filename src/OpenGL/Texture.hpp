#pragma once
#include <utility>
#include <glad/gl.h>
#include <glm/vec2.hpp>

class Texture2D
{
public:
    Texture2D() = default;
    Texture2D(GLsizei width, GLsizei height, GLenum format, GLsizei levels = 1);
    Texture2D(const glm::ivec2 &size, GLenum format, GLsizei levels = 1)
        : Texture2D(size.x, size.y, format, levels) { }
    Texture2D(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    
    ~Texture2D() noexcept;

    Texture2D& operator=(Texture2D&& other) noexcept;

    void Bind(GLuint unit) noexcept;
    void BindImage(GLuint unit, GLenum access) noexcept;

    constexpr GLuint GetID() const noexcept { return id_; }
    constexpr GLsizei GetWidth() const noexcept { return width_; }
    constexpr GLsizei GetHeight() const noexcept { return height_; }
    constexpr std::pair<GLsizei, GLsizei> GetSize() const noexcept { return {width_, height_}; }
private:
    GLuint id_;
    GLsizei width_;
    GLsizei height_;
    GLsizei levels_;
    GLenum format_;
};