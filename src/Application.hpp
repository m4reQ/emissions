#pragma once
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
    Buffer simConfigBuffer_;
    Buffer emittersBuffer_;
    Texture2D simOutputTexture_;
    Shader simComputeShader_;
    SimulationConfig simConfig_;
};