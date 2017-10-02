#pragma once

#include <Renderer/TextureManager.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

namespace engine {

class String;

class OPENGL_PLUGIN_API GL_TextureManagerDelegate
    : public TextureManagerDelegate {
public:
    GL_TextureManagerDelegate();

    ~GL_TextureManagerDelegate();

    Texture2D* CreateTexture2D() override;

    void DeleteTexture2D(Texture2D* shader) override;

    void SetActiveTexture2D(Texture2D* shader) override;
};

}  // namespace engine
