#pragma once

#include <Util/Prerequisites.hpp>

#include <memory>

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

    virtual bool initialize();

    virtual void shutdown();

    virtual void advanceFrame();

    virtual const String& getName() const = 0;

    RenderWindow& getRenderWindow();
    std::unique_ptr<RenderWindow>& getRenderWindowPtr();

protected:
    std::unique_ptr<RenderWindow> m_renderWindow;
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<ModelManager> m_modelManager;
};

}  // namespace engine
