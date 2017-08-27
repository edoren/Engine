#pragma once

#include <Renderer/Renderer.hpp>

#include "GL_Config.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    ~GL_Renderer();

    bool Initialize() override;

    void Shutdown() override;

    void AdvanceFrame() override;
};

}  // namespace engine
