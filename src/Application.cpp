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

static Shader CreateMainComputeShader(const Buffer& uniformBuffer)
{
    Shader mainComputeShader({ShaderStage{GL_COMPUTE_SHADER, "./data/shaders/MainCompute.glsl"}});
    mainComputeShader.BindUniformBuffer("uSimulationConfig", uniformBuffer);

    return mainComputeShader;
}

Application::Application()
{
    simConfig_ = {
        .EmissionRate = 2000.0f,
        .WindSpeed = 6.0f,
        .EffectiveHeight = 10.0f,
        .DepositionCoeff = 0.0001f,
        .XMax = 1000.0f,
        .YMax = 500.0f,
        .XRes = 512,
        .YRes = 512,
        // Stability class D
        .StabilityA = 0.08f,
        .StabilityB = 0.06f,
    };

    window_ = Window(1080, 720, "Emissions simulator");
    InitializeOpenGL();

    imguiContext_ = ImGUIContext(window_);
    simConfigBuffer_ = Buffer(sizeof(SimulationConfig));
    simOutputTexture_ = Texture2D(simConfig_.XRes, simConfig_.YRes, GL_R32F);
    simComputeShader_ = CreateMainComputeShader(simConfigBuffer_);
}

void Application::Run()
{
    double frameTime = 1.0;
    while (!window_.ShouldClose())
    {
        const auto start = window_.GetTime();

        window_.PollEvents();

        simConfigBuffer_.Write(simConfig_);
        simOutputTexture_.BindImage(1, GL_WRITE_ONLY);
        simComputeShader_.Use();
        
        glDispatchCompute(
            (simConfig_.XRes + 15) / 16,
            (simConfig_.YRes + 15) / 16,
            1);
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
