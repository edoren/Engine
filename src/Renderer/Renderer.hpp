#pragma once

#include <string>

#include <Renderer/RenderWindow.hpp>
#include <Graphics/Color.hpp>

namespace engine {

class Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize() = 0;

    virtual void ShutDown() = 0;

    virtual void AdvanceFrame();

    RenderWindow* GetRenderWindow() {
        return render_window_;
    };

protected:
    RenderWindow* render_window_;
};

}  // namespace engine
