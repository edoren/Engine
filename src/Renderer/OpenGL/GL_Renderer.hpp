#pragma once

#include <string>

#include <Util/Prerequisites.hpp>

#include <Renderer/Renderer.hpp>
#include <Renderer/OpenGL/GL_RenderWindow.hpp>

namespace engine {

class ENGINE_API GL_Renderer : public Renderer {
public:
    GL_Renderer();

    virtual ~GL_Renderer();

    virtual bool Initialize();

    virtual void ShutDown();

    virtual void AdvanceFrame();
};

}  // namespace engine
