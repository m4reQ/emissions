#pragma once
#include <cstdint>
#include <vector>
#include <utility>
#include <glad/gl.h>

struct FramebufferAttachment
{
    GLsizei Width;
    GLsizei Height;
    GLuint ID;
    bool IsResizable;
};

class Framebuffer
{
public:
    Framebuffer(GLsizei width, GLsizei height);
    Framebuffer(const std::pair<GLsizei, GLsizei> &size)
        : Framebuffer(size.first, size.second) { }

    ~Framebuffer() noexcept;

    void Bind() noexcept;
    void Unbind() noexcept;
    void AddAttachment(GLenum format, GLsizei width = 0, GLsizei height = 0);

    const FramebufferAttachment &GetAttachment(size_t index) const;
    constexpr GLsizei GetWidth() const noexcept { return width_; }
    constexpr GLsizei GetHeight() const noexcept { return height_; }
    
private:
    std::vector<FramebufferAttachment> attachments_;
    GLuint id_;
    GLsizei width_;
    GLsizei height_;
};