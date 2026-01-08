#include <iostream>
#include <stdexcept>
#include <format>
#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include "Window.hpp"
#include "OpenGL/Framebuffer.hpp"
#include "OpenGL/Shader.hpp"
#include "OpenGL/Buffer.hpp"
#include "OpenGL/Texture.hpp"

struct SimulationConfig
{
    float EmissionRate;
    float WindSpeed;
    float EffectiveHeight;
    float DepositionCoeff;
    float XMax;
    float YMax;
    int XRes;
    int YRes;
    float StabilityA;
    float StabilityB;
};

static void RenderMainFrame(Framebuffer &framebuffer)
{
    framebuffer.Bind();
    
    glClearColor(1.0, 1.0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    framebuffer.Unbind();
}

static void RenderImGUI(Framebuffer &framebuffer, Texture2D &simOutputTexture, double frameTime)
{
    const auto &colorAttachment = framebuffer.GetAttachment(0);

    ImGui::NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::DockSpaceOverViewport(0, 0, ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("Frame info");
    ImGui::Text(std::format("Frametime: {:.5f}", frameTime).c_str());
    ImGui::Text(std::format("FPS: {:.2f}", 1.0 / frameTime).c_str());
    ImGui::Text(std::format("Viewport size: {}x{}", colorAttachment.Width, colorAttachment.Height).c_str());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
    ImGui::Begin("Simulation output", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::Image((ImTextureRef)simOutputTexture.GetID(), ImVec2{(float)simOutputTexture.GetWidth(), (float)simOutputTexture.GetHeight()});
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void RunSimulation(Shader &computeShader, Texture2D &texture, Buffer &uniformBuffer, const SimulationConfig &config)
{
    uniformBuffer.Write(config);
    texture.BindImage(1, GL_WRITE_ONLY);
    computeShader.Use();

    glDispatchCompute(
        (config.XRes + 15) / 16,
        (config.YRes + 15) / 16,
        1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

static Shader CreateMainComputeShader(const Buffer& uniformBuffer)
{
    Shader mainComputeShader;
    mainComputeShader.AddStage(GL_COMPUTE_SHADER, std::filesystem::path("./data/shaders/MainCompute.glsl"));
    mainComputeShader.Link();
    mainComputeShader.BindUniformBuffer("uSimulationConfig", uniformBuffer);

    return mainComputeShader;
}

static Framebuffer CreateFramebuffer(const Window& window)
{
    Framebuffer framebuffer(window.GetSize());
    framebuffer.AddAttachment(GL_RGBA8);

    return framebuffer;
}

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

static void InitializeImGUI(const Window& window)
{
    if (!ImGui::CreateContext())
        throw std::runtime_error("Failed to create ImGUI context.");
    
    auto &imguiIO = ImGui::GetIO();

    const auto [width, height] = window.GetSize();
    imguiIO.DisplaySize = {(float)width, (float)height};

    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true))
        throw std::runtime_error("Failed to initialize ImGUI GLFW backend.");
    
    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("Failed to initialize ImGUI OpenGL backend.");
}

int main()
{
    SimulationConfig simulationConfig {
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

    Window window(1080, 720, "Emissions simulator");

    InitializeOpenGL();
    InitializeImGUI(window);

    Buffer uniformBuffer(sizeof(SimulationConfig));
    Texture2D simOutputTexture(simulationConfig.XRes, simulationConfig.YRes, GL_R32F);
    auto mainComputeShader = CreateMainComputeShader(uniformBuffer);
    auto framebuffer = CreateFramebuffer(window);
    
    double frameTime = 1.0;
    while (!window.ShouldClose())
    {
        const auto start = window.GetTime();

        window.PollEvents();

        RenderMainFrame(framebuffer);
        RunSimulation(mainComputeShader, simOutputTexture, uniformBuffer, simulationConfig);
        RenderImGUI(framebuffer, simOutputTexture, frameTime);

        window.SwapBuffers();

        frameTime = window.GetTime() - start;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    return 0;
}