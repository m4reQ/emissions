#pragma once
#include <vector>
#include <ImGuiFileDialog.h>
#include "EmitterInfo.hpp"
#include "SimulationConfig.hpp"
#include "Window.hpp"
#include "ImGUIContext.hpp"
#include "OpenGL/Buffer.hpp"
#include "OpenGL/Texture.hpp"
#include "OpenGL/Shader.hpp"

class Application
{
public:
    Application();

    void Run();
private:
    Window window_;
    ImGUIContext imguiContext_;
    IGFD::FileDialog fileOpenDialog_;
    Buffer simConfigBuffer_;
    Buffer emittersBuffer_;
    Texture2D simOutputTexture_;
    Shader simComputeShader_;
    SimulationConfig simConfig_;
    std::vector<EmitterInfo> simEmitters_;
    size_t selectedStabilityIdx_ = 0;
    size_t selectedEmitterIdx_ = 0;
    double frametime_ = 1.0;

    void CreateMainComputeShader();
    void RenderUI();
};