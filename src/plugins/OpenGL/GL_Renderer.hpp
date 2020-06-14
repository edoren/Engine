#pragma once

#include <Renderer/Renderer.hpp>

#include "GL_Config.hpp"

namespace engine {

class OPENGL_PLUGIN_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    ~GL_Renderer();

    bool initialize();

    void shutdown();

    void advanceFrame();

    const String& getName() const;
};

}  // namespace engine
