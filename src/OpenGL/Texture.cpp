#include "Texture.hpp"

Texture2D::Texture2D(GLsizei width, GLsizei height, GLenum format, GLsizei levels)
    : width_(width),
      height_(height),
      format_(format),
      levels_(levels)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &id_);
    glTextureStorage2D(id_, levels, format, width, height);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture2D::~Texture2D() noexcept
{
    glDeleteTextures(1, &id_);
}

void Texture2D::Bind(GLuint unit) noexcept
{
    glBindTextureUnit(unit, id_);
}

void Texture2D::BindImage(GLuint unit, GLenum access) noexcept
{
    glBindImageTexture(unit, id_, 0, GL_FALSE, 0, access, format_);
}