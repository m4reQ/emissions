#include "EmitterInfo.hpp" 

EmitterInfo EmitterInfo::FromJSON(const std::string_view data)
{
    return EmitterInfo::FromJSON(nlohmann::json::parse(data));
}

EmitterInfo EmitterInfo::FromJSON(const nlohmann::json& data)
{
    EmitterInfo info;
    data.at("position").at(0).get_to(info.Position[0]);
    data.at("position").at(1).get_to(info.Position[1]);
    data.at("emissionRate").get_to(info.EmissionRate);
    data.at("height").get_to(info.Height);

    return info;
}

EmitterInfo EmitterInfo::FromJSON(std::ifstream& fileStream)
{
    return EmitterInfo::FromJSON(nlohmann::json::parse(fileStream));
}

nlohmann::json EmitterInfo::ToJSON() const
{
    nlohmann::json json;
    json["position"] = nlohmann::json::array({Position.x, Position.y});
    json["emissionRate"] = EmissionRate;
    json["height"] = Height;

    return json;
}