#pragma once
#include <filesystem>
#include <string_view>
#include <vector>
#include <glad/gl.h>

class Shader
{
public:
    Shader();

    ~Shader() noexcept;

    void Use();
    void AddStage(GLenum type, const std::filesystem::path &filepath);
    void AddStage(GLenum type, const std::string_view source);
    void Link();

private:
    std::vector<GLuint> stages_;
    GLuint id_;
};