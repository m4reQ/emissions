#include "ImGUIContext.hpp"
#include <stdexcept>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

ImGUIContext::ImGUIContext(const Window &window)
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

void ImGUIContext::NewFrame()
{
    ImGui::NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
}

void ImGUIContext::Render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ImGUIContext::~ImGUIContext() noexcept
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
