#include "Application.hpp"
#include <iostream>
#include <array>
#include <ranges>
#include <format>
#include <utility>
#include <fstream>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
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
            std::cerr << "OpenGL message: " << message << '\n';
        },
        nullptr);
}

static std::pair<SimulationConfig, std::vector<EmitterInfo>> LoadSimulationConfigFromFile(const std::string_view filepath)
{
    std::ifstream file(filepath.data());
    if (!file.is_open())
        throw std::runtime_error("Failed to open simulation config file.");

    const auto data = nlohmann::json::parse(file);
    return std::make_pair(
        SimulationConfig::FromJSON(data),
        data.at("emitters")
            | std::views::transform([](const auto& x) { return EmitterInfo::FromJSON(x); })
            | std::ranges::to<std::vector>());
}

static void SaveSimulationConfigToFile(const std::string_view filepath, const SimulationConfig &config, const std::vector<EmitterInfo> &emitters)
{
    std::ofstream file(filepath.data());
    if (!file.is_open())
        throw std::runtime_error("Failed to open simulation config save file.");

    auto jsonConfig = config.ToJSON();
    jsonConfig["emitters"] = nlohmann::json::array();
    std::ranges::for_each(
        emitters,
        [&](const auto &x)
        {
            jsonConfig["emitters"].emplace_back(x.ToJSON());
        });
    
    jsonConfig >> file;
}

Application::Application()
{
    window_ = Window(1080, 720, "Emissions simulator", false);
    InitializeOpenGL();

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureResolution_);
    
    imguiContext_ = ImGUIContext(window_);
    simController_ = SimulationController({1000.0f, 500.0f}, {512, 512});
    gridResolutionNew_ = simController_.GetConfig().Resolution;
    gridSizeNew_ = simController_.GetConfig().Size;
}

void Application::Run()
{
    while (!window_.ShouldClose())
    {
        const auto start = window_.GetTime();

        window_.PollEvents();

        simController_.Calculate();

        RenderUI();

        window_.SwapBuffers();

        frametime_ = window_.GetTime() - start;
    }
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
                openFileDialogAction_ = OpenFileDialogAction::Open;
            }
            if (ImGui::MenuItem("Save Config...", "Ctrl+S"))
            {
                const IGFD::FileDialogConfig config {
                    .path = ".",
                    .countSelectionMax = 1,
                    .flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite,
                };
                fileOpenDialog_.OpenDialog("ChooseFileDlgKey", "Choose simulation config save file...", ".json", config);
                openFileDialogAction_ = OpenFileDialogAction::Save;
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
    ImGui::Text("Frametime: %.5lf", frametime_);
    ImGui::Text("FPS: %.2lf", 1.0 / frametime_);
    ImGui::End();

    ImGui::Begin("Simulation settings");
    ImGui::SliderFloat("Deposition coefficient", &simController_.GetConfig().DepositionCoeff, 0.0001f, 0.1f, "%.4f");

    const auto stability = simController_.GetConfig().Stability;
    const auto selectedStabilityIdx = std::distance(
        c_AtmosphericStabilityClasses.begin(),
        std::find_if(
            c_AtmosphericStabilityClasses.begin(),
            c_AtmosphericStabilityClasses.end(),
            [=](const auto &x)
            {
                return x.second == stability;
            }));
    if (ImGui::BeginCombo("Atmospheric stability", c_AtmosphericStabilityClasses[selectedStabilityIdx].first))
    {
        for (size_t i = 0; i < c_AtmosphericStabilityClasses.size(); i++)
        {
            const auto &stability = c_AtmosphericStabilityClasses[i];
            if (ImGui::Selectable(stability.first, selectedStabilityIdx == i))
                simController_.GetConfig().Stability = stability.second;
        }

        ImGui::EndCombo();
    }
    ImGui::SeparatorText("Wind");
    ImGui::SliderFloat("Speed [m/s]", &simController_.GetConfig().WindSpeed, 0.0f, 100.0f, "%.1f");

    // TODO Implement wind direction correctly
    ImGui::BeginDisabled();
    ImGui::SliderAngle("Direction", &simController_.GetConfig().WindDir);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Grid");
    ImGui::TextUnformatted("Resolution");
    ImGui::SliderInt("X", &gridResolutionNew_.x, 0, maxTextureResolution_, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt("Y", &gridResolutionNew_.y, 0, maxTextureResolution_, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextUnformatted("Size");
    ImGui::DragFloat("X [m]", &gridSizeNew_.x, 1.0f);
    ImGui::DragFloat("Y [m]", &gridSizeNew_.y, 1.0f);
    
    const auto gridResolutionChanged = gridResolutionNew_ != simController_.GetConfig().Resolution;
    const auto gridSizeChanged = 
        glm::epsilonNotEqual(gridSizeNew_.x, simController_.GetConfig().Size.x, 1.0e-6f)
        || glm::epsilonNotEqual(gridSizeNew_.y, simController_.GetConfig().Size.y, 1.0e-6f);
    ImGui::BeginDisabled(!(gridResolutionChanged || gridSizeChanged));
    if (ImGui::Button("Apply"))
    {
        simController_.GetConfig().Resolution = gridResolutionNew_;
        simController_.GetConfig().Size = gridSizeNew_;

        simController_.ResizeTexture(gridResolutionNew_);
    }
    ImGui::EndDisabled();

    ImGui::End();

    ImGui::Begin("Emitters");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginListBox("##Emitters"))
    {
        for (size_t i = 0; i < simController_.GetEmittersCount(); i++)
        {
            if (ImGui::Selectable(std::format("Emitter {}", i).c_str(), i == selectedEmitterIdx_))
                selectedEmitterIdx_ = i;
        }

        ImGui::EndListBox();
    }
    if (ImGui::Button("Add emitter"))
    {
        simController_.AddEmitter(EmitterInfo{});
    }
    ImGui::SameLine();
    ImGui::Text("Emitters count: %zu.", simController_.GetEmittersCount());
    ImGui::Separator();
    if (simController_.GetEmittersCount() > selectedEmitterIdx_)
    {
        auto &selectedEmitter = simController_.GetEmitter(selectedEmitterIdx_);
        const auto &simConfig = simController_.GetConfig();
        ImGui::Text("Position [m]");
        ImGui::DragFloat("X", &selectedEmitter.Position.x, 0.1f, 0.0f, simConfig.Size.x);
        ImGui::DragFloat("Y", &selectedEmitter.Position.y, 0.1f, 0.0f, simConfig.Size.y);
        ImGui::Separator();
        ImGui::DragFloat("Height [m]", &selectedEmitter.Height, 0.1f, 0.01f, 1000.0f, "%.1f");
        ImGui::DragFloat("Emission rate [g/s]", &selectedEmitter.EmissionRate, 1.0f, 0.0f, 0.0f, "%.0f");
        ImGui::Separator();

        if (ImGui::Button("Remove"))
            simController_.RemoveEmitter(selectedEmitterIdx_);
    }
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::Begin("Simulation output", nullptr, ImGuiWindowFlags_NoTitleBar);

    const auto& outputTexture = simController_.GetOutputTexture();
    const auto windowSize = ImGui::GetContentRegionAvail();
    const auto scale = std::min(
        windowSize.x / (float)outputTexture.GetWidth(),
        windowSize.y / (float)outputTexture.GetHeight());
    const ImVec2 textureSize{scale * outputTexture.GetWidth(), scale * outputTexture.GetHeight()};
    ImGui::SetCursorPosX((windowSize.x - textureSize.x) / 2);
    ImGui::SetCursorPosY((windowSize.y - textureSize.y) / 2);
    ImGui::Image((ImTextureRef)outputTexture.GetID(), textureSize);
    ImGui::End();
    ImGui::PopStyleVar();

    if (fileOpenDialog_.Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse))
    {
        if (fileOpenDialog_.IsOk())
        {
            if (openFileDialogAction_ == OpenFileDialogAction::Open)
            {
                auto [config, emitters] = LoadSimulationConfigFromFile(fileOpenDialog_.GetFilePathName());
                simController_.SetConfig(std::move(config));
                simController_.SetEmitters(std::move(emitters));
            }
            else
            {
                SaveSimulationConfigToFile(
                    fileOpenDialog_.GetFilePathName(),
                    simController_.GetConfig(),
                    simController_.GetEmitters());
            }
        }

        fileOpenDialog_.Close();
    }

    imguiContext_.Render();
}
