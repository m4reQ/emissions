#pragma once
#include "Window.hpp"

class ImGUIContext
{
public:
    ImGUIContext() = default;
    ImGUIContext(const Window &window);

    void NewFrame();
    void Render();

    ~ImGUIContext() noexcept;
};