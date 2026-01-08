#include "Application.hpp"
#include <iostream>
#include <imgui.h>

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

Application::Application()
{
    simConfig_ = {
        .Size = {1000.0f, 500.0f},
        .Stability = AtmosphericStabilityD,
        .WindSpeed = 6.0f,
        .DepositionCoeff = 0.0001f,
        .Resolution = {512, 512},
    };
    simEmitters_.emplace_back(
        EmitterInfo {
            .Position = {200.0f, 100.0f},
            .EmissionRate = 2000.0f,
            .Height = 10.0f});
    simEmitters_.emplace_back(
        EmitterInfo {
            .Position = {550.0f, 20.0f},
            .EmissionRate = 4000.0f,
            .Height = 8.0f});

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
    double frameTime = 1.0;
    while (!window_.ShouldClose())
    {
        const auto start = window_.GetTime();

        window_.PollEvents();

        simConfig_.EmittersCount = (int)simEmitters_.size();
        simConfigBuffer_.Write(&simConfig_, sizeof(SimulationConfig));
        emittersBuffer_.Write(simEmitters_.data(), sizeof(EmitterInfo) * simEmitters_.size());
        simOutputTexture_.BindImage(1, GL_WRITE_ONLY);
        simComputeShader_.Use();
        
        const auto groupSize = (simConfig_.Resolution + 15) / 16;
        glDispatchCompute(groupSize.x, groupSize.y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        imguiContext_.NewFrame();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::DockSpaceOverViewport(0, 0, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Frame info");
        ImGui::Text(std::format("Frametime: {:.5f}", frameTime).c_str());
        ImGui::Text(std::format("FPS: {:.2f}", 1.0 / frameTime).c_str());
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::Begin("Simulation output", nullptr, ImGuiWindowFlags_NoTitleBar);
        ImGui::Image((ImTextureRef)simOutputTexture_.GetID(), ImVec2{(float)simOutputTexture_.GetWidth(), (float)simOutputTexture_.GetHeight()});
        ImGui::End();
        ImGui::PopStyleVar();

        imguiContext_.Render();

        window_.SwapBuffers();

        frameTime = window_.GetTime() - start;
    }
}

void Application::CreateMainComputeShader()
{
    simComputeShader_ = Shader({ShaderStage{GL_COMPUTE_SHADER, "./data/shaders/MainCompute.glsl"}});
    simComputeShader_.BindUniformBuffer(1, simConfigBuffer_);
    simComputeShader_.BindShaderStorageBuffer(2, emittersBuffer_);
}
