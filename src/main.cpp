#include <iostream>
#include <stdexcept>
#include <format>
#include "Window.hpp"
#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

int main()
{
    Window window(1080, 720, "Emissions simulator");

    if (!gladLoadGL(glfwGetProcAddress))
        throw std::runtime_error("Failed to load OpenGL bindings.");

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
    
    while (!window.ShouldClose())
    {
        window.PollEvents();

        ImGui::NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();

        glClearColor(1.0, 1.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::DockSpaceOverViewport(0, 0, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Test window");
        ImGui::Text("Hello world!");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.SwapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    return 0;
}