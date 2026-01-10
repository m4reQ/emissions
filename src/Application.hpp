#pragma once
#include <vector>
#include <ImGuiFileDialog.h>
#include "Window.hpp"
#include "ImGUIContext.hpp"
#include "SimulationController.hpp"

class Application
{
public:
    Application();

    void Run();
private:
    Window window_;
    ImGUIContext imguiContext_;
    IGFD::FileDialog fileOpenDialog_;
    SimulationController simController_;
    size_t selectedEmitterIdx_ = 0;
    double frametime_ = 1.0;

    void CreateMainComputeShader();
    void RenderUI();
};