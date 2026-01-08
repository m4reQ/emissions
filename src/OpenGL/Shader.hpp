#pragma once
#include <filesystem>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <glad/gl.h>
#include "Buffer.hpp"
#include "../StringHash.hpp"

struct ShaderStage
{
    GLenum Type;
    const std::string_view SourceOrFilepath;
    bool IsFromFile = true;
};

class Shader
{
public:
    Shader() = default;
    Shader(const std::vector<ShaderStage> &stages);
    Shader(const Shader&) = delete;
    Shader(Shader&& other) noexcept;

    ~Shader() noexcept;

    Shader& operator=(Shader&& other) noexcept;

    void Use();
    void BindUniformBuffer(const std::string_view uniformBlockName, const Buffer &buffer);
    void BindUniformBuffer(GLuint binding, const Buffer &buffer);
    void BindShaderStorageBuffer(GLuint binding, const Buffer &buffer);

    constexpr GLuint GetID() const noexcept { return id_; }

private:
    std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> interface_;
    GLuint id_;

    GLuint GetUniformBlockLocation(const std::string_view name);
};