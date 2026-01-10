#include "Application.hpp"
#include <iostream>
#include <array>
#include <utility>
#include <fstream>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>

constexpr std::array<std::pair<const char*, glm::vec2>, 6> c_AtmosphericStabilityClasses {
    std::make_pair("Extremely unstable (A)", AtmosphericStabilityA),
    std::make_pair("Moderately unstable (B)", AtmosphericStabilityB),
    std::make_pair("Slightly unstable (C)", AtmosphericStabilityC),
    std::make_pair("Neutral (D)", AtmosphericStabilityD),
    std::make_pair("Slightly stable (E)", AtmosphericStabilityE),
    std::make_pair("Moderately stable (F)", AtmosphericStabilityF),
};

static void InitializeOpenGL()
{
    if (!gladLoadGL(glfwGetProcAddress))
        throw std::runtime_error("Failed to load OpenGL bindings.");

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
        {
            std::cerr << std::format("OpenGL message: {}\n", message);
        },
        nullptr);
}

static SimulationConfig LoadSimulationConfigFromFile(const std::string_view filepath, std::vector<EmitterInfo>& emitters)
{
    std::ifstream file(filepath.data());
    if (!file.is_open())
        throw std::runtime_error("Failed to open simulation config file.");

    nlohmann::json data = nlohmann::json::parse(file);
    
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

    for (const auto &emitterData : data.at("emitters"))
    {
        EmitterInfo info;
        emitterData.at("position").at(0).get_to(info.Position[0]);
        emitterData.at("position").at(1).get_to(info.Position[1]);
        emitterData.at("emissionRate").get_to(info.EmissionRate);
        emitterData.at("height").get_to(info.Height);

        emitters.emplace_back(info);
    }

    return config;
}

Application::Application()
{
    simConfig_ = {
        .Size = {1000.0f, 500.0f},
        .Stability = AtmosphericStabilityD,
        .WindSpeed = 10.0f,
        .WindDir = glm::radians(0.0f),
        .DepositionCoeff = 0.0001f,
        .Resolution = {512, 512},
    };

    window_ = Window(1080, 720, "Emissions simulator");
    InitializeOpenGL();

    imguiContext_ = ImGUIContext(window_);
    simConfigBuffer_ = Buffer(sizeof(SimulationConfig));
    emittersBuffer_ = Buffer(32 * sizeof(EmitterInfo));
    simOutputTexture_ = Texture2D(simConfig_.Resolution.x, simConfig_.Resolution.y, GL_R32F);
    CreateMainComputeShader();
}

void Application::Run()
{
    while (!window_.ShouldClose())
    {
        const auto start = window_.GetTime();

        window_.PollEvents();

        simConfig_.EmittersCount = (int)simEmitters_.size();
        simConfig_.Stability = c_AtmosphericStabilityClasses[selectedStabilityIdx_].second;
        simConfigBuffer_.Write(&simConfig_, sizeof(SimulationConfig));
        emittersBuffer_.Write(simEmitters_.data(), sizeof(EmitterInfo) * simEmitters_.size());
        simOutputTexture_.BindImage(1, GL_WRITE_ONLY);
        simComputeShader_.Use();
        
        const auto groupSize = (simConfig_.Resolution + 15) / 16;
        glDispatchCompute(groupSize.x, groupSize.y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        RenderUI();

        window_.SwapBuffers();

        frametime_ = window_.GetTime() - start;
    }
}

void Application::CreateMainComputeShader()
{
    simComputeShader_ = Shader({ShaderStage{GL_COMPUTE_SHADER, "./data/shaders/MainCompute.glsl"}});
    simComputeShader_.BindUniformBuffer(1, simConfigBuffer_);
    simComputeShader_.BindShaderStorageBuffer(2, emittersBuffer_);
}

void Application::RenderUI()
{
    imguiContext_.NewFrame();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Config...", "Ctrl+O"))
            {
                const IGFD::FileDialogConfig config {
                    .path = ".",
                    .countSelectionMax = 1,
                    .flags = ImGuiFileDialogFlags_Modal,
                };
                fileOpenDialog_.OpenDialog("ChooseFileDlgKey", "Choose simulation config file...", ".json", config);
            }
            if (ImGui::MenuItem("Save Config...", "Ctrl+S"))
            {
                // TODO Add ability to save simulation config to file
            }
            if (ImGui::MenuItem("Save Config As...", "Ctrl+Shift+S"))
            {
                // TODO Add ability to save simulation config to file
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close", "Alt+F4"))
                window_.Close();

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::DockSpaceOverViewport(0, 0, ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Frame info");
    ImGui::Text(std::format("Frametime: {:.5f}", frametime_).c_str());
    ImGui::Text(std::format("FPS: {:.2f}", 1.0 / frametime_).c_str());
    ImGui::End();

    ImGui::Begin("Simulation settings");
    ImGui::SliderFloat("Deposition coefficient", &simConfig_.DepositionCoeff, 0.0001f, 0.1f, "%.4f");
    if (ImGui::BeginCombo("Atmospheric stability", c_AtmosphericStabilityClasses[selectedStabilityIdx_].first))
    {
        for (size_t i = 0; i < c_AtmosphericStabilityClasses.size(); i++)
        {
            const auto &stability = c_AtmosphericStabilityClasses[i];
            if (ImGui::Selectable(stability.first, selectedStabilityIdx_ == i))
                selectedStabilityIdx_ = i;
        }

        ImGui::EndCombo();
    }
    ImGui::SeparatorText("Wind");
    ImGui::SliderFloat("Speed [m/s]", &simConfig_.WindSpeed, 0.0f, 100.0f, "%.1f");
    ImGui::SliderAngle("Direction", &simConfig_.WindDir);
    ImGui::End();

    ImGui::Begin("Emitters");
    ImGui::Text("Emitters count: %d.", simEmitters_.size());
    if (ImGui::BeginListBox("##Emitters"))
    {
        for (size_t i = 0; i < simEmitters_.size(); i++)
        {
            if (ImGui::Selectable(std::format("Emitter {}", i).c_str(), i == selectedEmitterIdx_))
                selectedEmitterIdx_ = i;
        }

        ImGui::EndListBox();
    }
    ImGui::End();

    ImGui::Begin("Emitter info");
    if (simEmitters_.size() > selectedEmitterIdx_)
    {
        auto &selectedEmitter = simEmitters_[selectedEmitterIdx_];
        ImGui::Text("Position [m]");
        ImGui::DragFloat("X", &selectedEmitter.Position.x, 0.1f, 0.0f, simConfig_.Size.x);
        ImGui::DragFloat("Y", &selectedEmitter.Position.y, 0.1f, 0.0f, simConfig_.Size.y);
        ImGui::Separator();
        ImGui::DragFloat("Height [m]", &selectedEmitter.Height, 0.1f, 0.01f, 1000.0f, "%.1f");
        ImGui::DragFloat("Emission rate [g/s]", &selectedEmitter.EmissionRate, 1.0f, 0.0f, 0.0f, "%.0f");
    }
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::Begin("Simulation output", nullptr, ImGuiWindowFlags_NoTitleBar);
    const ImVec2 textureSize{(float)simOutputTexture_.GetWidth(), (float)simOutputTexture_.GetHeight()};
    const auto windowSize = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPosX((windowSize.x - textureSize.x) / 2);
    ImGui::SetCursorPosY((windowSize.y - textureSize.y) / 2);
    ImGui::Image((ImTextureRef)simOutputTexture_.GetID(), textureSize);
    ImGui::End();
    ImGui::PopStyleVar();

    if (fileOpenDialog_.Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse))
    {
        if (fileOpenDialog_.IsOk())
        {
            simEmitters_.clear();
            simConfig_ = LoadSimulationConfigFromFile(fileOpenDialog_.GetFilePathName(), simEmitters_);
        }

        fileOpenDialog_.Close();
    }

    imguiContext_.Render();
}
