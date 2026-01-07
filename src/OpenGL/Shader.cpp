#include "Shader.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <format>

Shader::Shader()
{
    id_ = glCreateProgram();
}

Shader::~Shader() noexcept
{
    glDeleteProgram(id_);
}

void Shader::Use()
{
    glUseProgram(id_);
}

void Shader::AddStage(GLenum type, const std::filesystem::path &filepath)
{
    std::ifstream file;
    file.open(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader stage source file.");

    std::vector<char> source(2048);

    size_t offset = 0;
    while (true)
    {
        file.read(source.data() + offset, 2048);
        offset += (size_t)file.gcount();
        if (offset >= source.max_size())
            source.reserve(source.max_size() + 2048);
        else
            break;
    }

    source.resize(offset);

    AddStage(type, std::string_view{source.data(), offset});
}

void Shader::AddStage(GLenum type, const std::string_view source)
{
    GLuint stage = glCreateShader(type);

    const auto sourceData = source.data();
    const auto sourceLength = (GLsizei)source.size();
    glShaderSource(stage, 1, &sourceData, &sourceLength);

    glCompileShader(stage);

    GLint logLength = 0;
    glGetShaderiv(stage, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength != 0)
    {
        std::string infoLog(logLength, '\0');
        glGetShaderInfoLog(stage, infoLog.size() * sizeof(char), nullptr, infoLog.data());

        std::cerr << std::format("Failed to compile shader stage:\n{}\n", infoLog);
        throw std::runtime_error("Failed to compile shader stage.");
    }
    
    glAttachShader(id_, stage);

    stages_.emplace_back(stage);
}

void Shader::Link()
{
    glLinkProgram(id_);

    for (const auto stageID : stages_)
    {
        glDetachShader(id_, stageID);
        glDeleteShader(stageID);
    }

    GLint logLength = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength != 0)
    {
        std::string infoLog(logLength, '\0');
        glGetProgramInfoLog(id_, infoLog.size() * sizeof(char), nullptr, infoLog.data());

        std::cerr << std::format("Failed to link shader:\n{}\n", infoLog);
        throw std::runtime_error("Failed to link shader.");
    }
}
