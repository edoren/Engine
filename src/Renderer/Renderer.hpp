#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/RenderWindow.hpp>
#include <Graphics/Color.hpp>

namespace engine {

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize() = 0;

    virtual void ShutDown() = 0;

    virtual void AdvanceFrame();

    RenderWindow& GetRenderWindow();

    RenderWindow* GetRenderWindowPtr();

protected:
    RenderWindow* render_window_;
};

}  // namespace engine
