#include "SimulationConfig.hpp"

SimulationConfig SimulationConfig::FromJSON(const std::string_view data)
{
    return FromJSON(nlohmann::json::parse(data));
}

SimulationConfig SimulationConfig::FromJSON(const nlohmann::json& data)
{
    SimulationConfig config;
    data.at("size").at(0).get_to(config.Size[0]);
    data.at("size").at(1).get_to(config.Size[1]);
    data.at("stability").at(0).get_to(config.Stability[0]);
    data.at("stability").at(1).get_to(config.Stability[1]);
    data.at("windSpeed").get_to(config.WindSpeed);
    data.at("windDir").get_to(config.WindDir);
    data.at("depositionCoeff").get_to(config.DepositionCoeff);
    data.at("resolution").at(0).get_to(config.Resolution[0]);
    data.at("resolution").at(1).get_to(config.Resolution[1]);

    return config;
}

SimulationConfig SimulationConfig::FromJSON(std::ifstream& fileStream)
{
    return FromJSON(nlohmann::json::parse(fileStream));
}

nlohmann::json SimulationConfig::ToJSON() const
{
    nlohmann::json json;
    json["size"] = nlohmann::json::array({Size.x, Size.y});
    json["stability"] = nlohmann::json::array({Stability.x, Stability.y});
    json["windSpeed"] = WindSpeed;
    json["windDir"] = WindDir;
    json["depositionCoeff"] = DepositionCoeff;
    json["resolution"] = nlohmann::json::array({Resolution.x, Resolution.y});

    return json;
}
