#pragma once
#include <string_view>
#include <utility>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window() = default;
    Window(int32_t width, int32_t height, const std::string_view name, bool enableVsync = true);

    ~Window() noexcept;

    bool ShouldClose() const noexcept;
    void PollEvents() const noexcept;
    void SwapBuffers() const noexcept;
    int32_t GetWidth() const noexcept;
    int32_t GetHeight() const noexcept;
    std::pair<int32_t, int32_t> GetSize() const noexcept;
    double GetTime() const noexcept;

    constexpr GLFWwindow *GetHandle() const noexcept { return window_; }

private:
    GLFWwindow *window_;
};