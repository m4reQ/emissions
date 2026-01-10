#pragma once
#include <string_view>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/vec2.hpp>

struct EmitterInfo
{
    glm::vec2 Position;
    float EmissionRate;
    float Height;

    static EmitterInfo FromJSON(const std::string_view data);
    static EmitterInfo FromJSON(const nlohmann::json& data);
    static EmitterInfo FromJSON(std::ifstream& fileStream);
};