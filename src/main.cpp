#include <iostream>
#include <stdexcept>
#include <format>
#include "Window.hpp"
#include "OpenGL/Framebuffer.hpp"
#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

static void RenderMainFrame(Framebuffer &framebuffer)
{
    framebuffer.Bind();
    
    glClearColor(1.0, 1.0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    framebuffer.Unbind();
}

static void RenderImGUI(Framebuffer &framebuffer, double frameTime)
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
    ImGui::Begin("Framebuffer", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::Image((ImTextureRef)colorAttachment.ID, ImGui::GetContentRegionAvail());
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main()
{
    Window window(1080, 720, "Emissions simulator");

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

    Framebuffer framebuffer(window.GetSize());
    framebuffer.AddAttachment(GL_RGBA8);

    if (!ImGui::CreateContext())
        throw std::runtime_error("Failed to create ImGUI context.");

    const auto [width, height] = window.GetSize();
    auto &imguiIO = ImGui::GetIO();
    imguiIO.DisplaySize = {(float)width, (float)height};
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true))
        throw std::runtime_error("Failed to initialize ImGUI GLFW backend.");
    
    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("Failed to initialize ImGUI OpenGL backend.");
    
    double frameTime = 1.0;
    while (!window.ShouldClose())
    {
        const auto start = window.GetTime();

        window.PollEvents();

        RenderMainFrame(framebuffer);
        RenderImGUI(framebuffer, frameTime);

        window.SwapBuffers();

        frameTime = window.GetTime() - start;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    return 0;
}