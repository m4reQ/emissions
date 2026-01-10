#include "SimulationController.hpp"
#include <glm/glm.hpp>

constexpr glm::vec2 c_DefaultAtmosphericStability = AtmosphericStabilityD;
constexpr float c_DefaultWindSpeed = 10.0f;
constexpr float c_DefaultWindDir = glm::radians(0.0f);
constexpr float c_DefaultDepositionCoeff = 0.0001f;
constexpr size_t c_DefaultEmittersCapacity = 32;
constexpr GLuint c_ConfigBufferBinding = 1;
constexpr GLuint c_EmittersBufferBinding = 2;
constexpr GLuint c_OutputTextureBinding = 1;

SimulationController::SimulationController(const glm::vec2 &gridSize, const glm::ivec2 &gridResolution)
{
    config_ = {
        .Size = gridSize,
        .Stability = c_DefaultAtmosphericStability,
        .WindSpeed = c_DefaultWindSpeed,
        .WindDir = c_DefaultWindDir,
        .DepositionCoeff = c_DefaultDepositionCoeff,
        .Resolution = gridResolution,
    };

    emitters_.reserve(c_DefaultEmittersCapacity);

    configBuffer_ = Buffer(sizeof(SimulationConfig));
    emittersBuffer_ = Buffer(sizeof(EmitterInfo) * c_DefaultEmittersCapacity);
    outputTexture_ = Texture2D(gridResolution, GL_R32F);

    computeShader_ = Shader({{GL_COMPUTE_SHADER, "./data/shaders/MainCompute.glsl"}});
    computeShader_.BindUniformBuffer(c_ConfigBufferBinding, configBuffer_);
    computeShader_.BindShaderStorageBuffer(c_EmittersBufferBinding, emittersBuffer_);
}

SimulationController::SimulationController(SimulationController &&other) noexcept
{
    config_ = std::move(other.config_);
    emitters_ = std::move(other.emitters_);
    configBuffer_ = std::move(other.configBuffer_);
    emittersBuffer_ = std::move(other.emittersBuffer_);
    outputTexture_ = std::move(other.outputTexture_);
    computeShader_ = std::move(other.computeShader_);
}

SimulationController &SimulationController::operator=(SimulationController &&other) noexcept
{
    config_ = std::move(other.config_);
    emitters_ = std::move(other.emitters_);
    configBuffer_ = std::move(other.configBuffer_);
    emittersBuffer_ = std::move(other.emittersBuffer_);
    outputTexture_ = std::move(other.outputTexture_);
    computeShader_ = std::move(other.computeShader_);

    return *this;
}

void SimulationController::Calculate()
{
    const auto emittersCount = emitters_.size();
    if (emittersCount * sizeof(EmitterInfo) > emittersBuffer_.GetSize())
    {
        emittersBuffer_ = Buffer(sizeof(EmitterInfo) * emitters_.capacity());
        computeShader_.BindShaderStorageBuffer(c_EmittersBufferBinding, emittersBuffer_);
    }

    emittersBuffer_.Write(emitters_.data(), sizeof(EmitterInfo) * emittersCount);

    config_.EmittersCount = (int)emittersCount;
    configBuffer_.Write(&config_, sizeof(SimulationConfig));

    outputTexture_.BindImage(c_OutputTextureBinding, GL_WRITE_ONLY);

    computeShader_.Use();

    const auto groupSize = (config_.Resolution + 15) / 16;
    glDispatchCompute(groupSize.x, groupSize.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SimulationController::AddEmitter(EmitterInfo &&emitterInfo)
{
    emitters_.emplace_back(std::forward<EmitterInfo>(emitterInfo));
}

void SimulationController::AddEmitter(const glm::vec2 &position, float height, float emissionRate)
{
    AddEmitter(EmitterInfo{.Position = position, .EmissionRate = emissionRate, .Height = height});
}

void SimulationController::RemoveEmitter(size_t emitterIdx)
{
    emitters_.erase(emitters_.begin() + emitterIdx);
}

void SimulationController::ClearEmitters()
{
    emitters_.clear();
}