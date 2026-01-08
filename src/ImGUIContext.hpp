#pragma once
#include "Window.hpp"

class ImGUIContext
{
public:
    ImGUIContext() = default;
    ImGUIContext(const Window &window);
    ImGUIContext(const ImGUIContext&) = delete;
    ImGUIContext(ImGUIContext&& other) noexcept;

    ~ImGUIContext() noexcept;

    ImGUIContext& operator=(ImGUIContext&& other) noexcept;
    
    void NewFrame();
    void Render();

private:
    bool shouldFree_ = true;
};