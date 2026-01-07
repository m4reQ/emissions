#pragma once
#include <filesystem>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <glad/gl.h>
#include "Buffer.hpp"
#include "../StringHash.hpp"

class Shader
{
public:
    Shader();

    ~Shader() noexcept;

    void Use();
    void AddStage(GLenum type, const std::filesystem::path &filepath);
    void AddStage(GLenum type, const std::string_view source);
    void Link();
    void BindUniformBuffer(const std::string_view uniformBlockName, const Buffer &buffer);

    constexpr GLuint GetID() const noexcept { return id_; }

private:
    std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> interface_;
    std::vector<GLuint> stages_;
    GLuint id_;

    GLuint GetUniformBlockLocation(const std::string_view name);
};