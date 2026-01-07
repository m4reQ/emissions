#include "Window.hpp"
#include <stdexcept>
#include <iostream>
#include <format>

Window::Window(int32_t width, int32_t height, const std::string_view name)
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, _DEBUG);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    window_ = glfwCreateWindow(1080, 720, "Emissions simulator", nullptr, nullptr);
    if (!window_)
        throw std::runtime_error("Failed to create GLFW window.");

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
}

Window::~Window() noexcept
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::ShouldClose() const noexcept
{
    return glfwWindowShouldClose(window_);
}

void Window::PollEvents() const noexcept
{
    glfwPollEvents();
}

void Window::SwapBuffers() const noexcept
{
    glfwSwapBuffers(window_);
}

int32_t Window::GetWidth() const noexcept
{
    int32_t width;
    glfwGetFramebufferSize(window_, &width, nullptr);

    return width;
}

int32_t Window::GetHeight() const noexcept
{
    int32_t height;
    glfwGetFramebufferSize(window_, nullptr, &height);

    return height;
}

std::pair<int32_t, int32_t> Window::GetSize() const noexcept
{
    int32_t width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    return {width, height};
}

double Window::GetTime() const noexcept
{
    return glfwGetTime();
}
