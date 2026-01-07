#pragma once
#include <string_view>
#include <utility>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window() = default;
    Window(int32_t width, int32_t height, const std::string_view name);

    ~Window() noexcept;

    bool ShouldClose() noexcept;
    void PollEvents() noexcept;
    void SwapBuffers() noexcept;
    int32_t GetWidth() noexcept;
    int32_t GetHeight() noexcept;
    std::pair<int32_t, int32_t> GetSize() noexcept;

    constexpr GLFWwindow *GetHandle() const noexcept { return window_; }

private:
    GLFWwindow *window_;
};