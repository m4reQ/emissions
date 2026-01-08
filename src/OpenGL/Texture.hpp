#pragma once
#include <utility>
#include <glad/gl.h>

class Texture2D
{
public:
    Texture2D() = default;
    Texture2D(GLsizei width, GLsizei height, GLenum format, GLsizei levels = 1);
    
    ~Texture2D() noexcept;

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