#pragma once

#include <Renderer/Renderer.hpp>

#include "GL_Config.hpp"

namespace engine::plugin::opengl {

class OPENGL_PLUGIN_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    ~GL_Renderer() override;

    bool initialize() override;

    void shutdown() override;

    void advanceFrame() override;

    const String& getName() const override;
};

}  // namespace engine::plugin::opengl
