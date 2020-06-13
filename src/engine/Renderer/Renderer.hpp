#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

class RenderWindow;
class ShaderManager;
class String;
class TextureManager;
class ModelManager;

class ENGINE_API Renderer {
public:
    Renderer();

    virtual ~Renderer();

    virtual bool Initialize();

    virtual void Shutdown();

    virtual void AdvanceFrame();

    virtual const String& GetName() const = 0;

    RenderWindow& GetRenderWindow();
    std::unique_ptr<RenderWindow>& GetRenderWindowPtr();

protected:
    std::unique_ptr<RenderWindow> m_render_window;
    std::unique_ptr<ShaderManager> m_shader_manager;
    std::unique_ptr<TextureManager> m_texture_manager;
    std::unique_ptr<ModelManager> m_model_manager;
};

}  // namespace engine
