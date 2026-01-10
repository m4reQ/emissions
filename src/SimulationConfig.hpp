#pragma once
#include <string_view>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/vec2.hpp>

constexpr glm::vec2 AtmosphericStabilityA {0.22f, 0.20f};
constexpr glm::vec2 AtmosphericStabilityB {0.16f, 0.12f};
constexpr glm::vec2 AtmosphericStabilityC {0.11f, 0.08f};
constexpr glm::vec2 AtmosphericStabilityD {0.08f, 0.06f};
constexpr glm::vec2 AtmosphericStabilityE {0.06f, 0.03f};
constexpr glm::vec2 AtmosphericStabilityF {0.04f, 0.016f};

struct SimulationConfig
{
    glm::vec2 Size;
    glm::vec2 Stability;
    float WindSpeed;
    float WindDir;
    float DepositionCoeff;
    float _Pad1;
    glm::ivec2 Resolution;
    int EmittersCount;

    static SimulationConfig FromJSON(const std::string_view data);
    static SimulationConfig FromJSON(const nlohmann::json& data);
    static SimulationConfig FromJSON(std::ifstream& fileStream);
};