#include "Framebuffer.hpp"

Framebuffer::Framebuffer(GLsizei width, GLsizei height)
    : width_(width), height_(height)
{
    glCreateFramebuffers(1, &id_);
}

Framebuffer::~Framebuffer() noexcept
{
    std::vector<GLuint> attachmentIDs;
    attachmentIDs.reserve(attachments_.size());
    for (const auto& attachment : attachments_)
        attachmentIDs.emplace_back(attachment.ID);

    glDeleteTextures(attachmentIDs.size(), attachmentIDs.data());
    glDeleteFramebuffers(1, &id_);
}

void Framebuffer::Bind() noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void Framebuffer::Unbind() noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AddAttachment(GLenum format, GLsizei width, GLsizei height)
{
    const bool isResizable = width != 0 && height != 0;
    if (!isResizable)
    {
        width = width_;
        height = height_;
    }

    GLuint attachmentID = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &attachmentID);
    glTextureStorage2D(attachmentID, 1, format, width, height);
    glTextureParameteri(attachmentID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(attachmentID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(attachmentID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(attachmentID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const auto attachmentIndex = GL_COLOR_ATTACHMENT0 + attachments_.size();
    glNamedFramebufferTexture(id_, attachmentIndex, attachmentID, 0);
    glNamedFramebufferDrawBuffer(id_, attachmentIndex);

    attachments_.emplace_back(FramebufferAttachment{width, height, attachmentID, isResizable});
}

const FramebufferAttachment &Framebuffer::GetAttachment(size_t index) const
{
    return attachments_.at(index);
}
