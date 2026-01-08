#include "Shader.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <format>
#include <ranges>

static GLuint CreateStage(GLenum type, const std::string_view source)
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

    return stage;
}

static GLuint CreateStageFromFile(GLenum type, const std::string_view filepath)
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

    return CreateStage(type, std::string_view{source.data(), offset});
}

Shader::Shader(const std::vector<ShaderStage> &stages)
{
    id_ = glCreateProgram();

    auto stageIDs = stages
        | std::views::transform(
            [](const auto &stage)
            {
                return stage.IsFromFile
                    ? CreateStageFromFile(stage.Type, stage.SourceOrFilepath)
                    : CreateStage(stage.Type, stage.SourceOrFilepath);
            })
        | std::ranges::to<std::vector>();
    
    for (const auto stageID : stageIDs)
        glAttachShader(id_, stageID);
    
    glLinkProgram(id_);

    for (const auto stageID : stageIDs)
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

Shader::Shader(Shader &&other) noexcept
{
    id_ = std::exchange(other.id_, 0);
    interface_ = std::move(other.interface_);
}

Shader::~Shader() noexcept
{
    glDeleteProgram(id_);
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    id_ = std::exchange(other.id_, 0);
    interface_ = std::move(other.interface_);
    return *this;
}

void Shader::Use()
{
    glUseProgram(id_);
}

void Shader::BindUniformBuffer(const std::string_view uniformBlockName, const Buffer &buffer)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, GetUniformBlockLocation(uniformBlockName), buffer.GetID());
}

GLuint Shader::GetUniformBlockLocation(const std::string_view name)
{
    const auto it = interface_.find(name);
    if (it != interface_.end())
        return it->second;
    
    GLuint id = glGetUniformBlockIndex(id_, name.data());
    interface_.emplace(name, id);

    return id;
}
