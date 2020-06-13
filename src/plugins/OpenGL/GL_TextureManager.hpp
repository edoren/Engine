#pragma once

#include <Renderer/TextureManager.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class String;

class OPENGL_PLUGIN_API GL_TextureManager : public TextureManager {
public:
    GL_TextureManager();

    ~GL_TextureManager();

    static GL_TextureManager& GetInstance();

    static GL_TextureManager* GetInstancePtr();

protected:
    std::unique_ptr<Texture2D> CreateTexture2D() override;
    void UseTexture2D(Texture2D* texture) override;
};

}  // namespace engine
