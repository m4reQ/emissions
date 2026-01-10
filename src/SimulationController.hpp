#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include "SimulationConfig.hpp"
#include "EmitterInfo.hpp"
#include "OpenGL/Buffer.hpp"
#include "OpenGL/Texture.hpp"
#include "OpenGL/Shader.hpp"

class SimulationController
{
public:
    SimulationController() = default;
    SimulationController(const SimulationController&) = delete;
    SimulationController(const glm::vec2 &gridSize, const glm::ivec2 &gridResolution);
    SimulationController(SimulationController &&other) noexcept;

    SimulationController& operator=(SimulationController &&other) noexcept;

    void Calculate();
    void AddEmitter(EmitterInfo&& emitterInfo);
    void AddEmitter(const glm::vec2 &position, float height, float emissionRate);
    void RemoveEmitter(size_t emitterIdx);
    void ClearEmitters();
    void SetEmitters(std::vector<EmitterInfo> &&emitters) noexcept { emitters_ = std::move(emitters); }
    void SetConfig(SimulationConfig &&config) noexcept { config_ = std::move(config); }

    constexpr SimulationConfig& GetConfig() noexcept { return config_; }
    constexpr std::vector<EmitterInfo>& GetEmitters() noexcept { return emitters_; }
    constexpr EmitterInfo& GetEmitter(size_t emitterIdx) noexcept { return emitters_.at(emitterIdx); }
    constexpr size_t GetEmittersCount() const noexcept { return emitters_.size(); }
    constexpr const Texture2D& GetOutputTexture() const noexcept { return outputTexture_; }

private:
    SimulationConfig config_;
    std::vector<EmitterInfo> emitters_;
    Buffer configBuffer_;
    Buffer emittersBuffer_;
    Texture2D outputTexture_;
    Shader computeShader_;
};