#include <iostream>
#include <stdexcept>
#include <format>
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

int main()
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW.");

    glfwSetErrorCallback(
        [](int errorCode, const char *error)
        {
            std::cerr << std::format("GLFW error: {} ({}).\n", error, errorCode);
        });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(1080, 720, "Emissions simulator", nullptr, nullptr);
    if (!window)
        throw std::runtime_error("Failed to create GLFW window.");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress))
        throw std::runtime_error("Failed to load OpenGL bindings.");

    if (!ImGui::CreateContext())
        throw std::runtime_error("Failed to create ImGUI context.");

    ImGui::GetIO().DisplaySize = {1080.0f, 720.0f};

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        throw std::runtime_error("Failed to initialize ImGUI GLFW backend.");
    
    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("Failed to initialize ImGUI OpenGL backend.");
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui::NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();

        glClearColor(1.0, 1.0, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Begin("Test window");
        ImGui::Text("Hello world!");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}