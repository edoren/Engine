#pragma once

#include <Core/Plugin.hpp>

#include "GL_Config.hpp"

namespace engine::plugin::opengl {

class GL_Renderer;
class GL_ShaderManagerDelegate;
class GL_TextureManagerDelegate;

/**
 * @brief OpenGL plugin class
 */
class OPENGL_PLUGIN_API GL_Plugin : public Plugin {
public:
    GL_Plugin();

    const String& getName() const override;

    void install() override;

    void initialize() override;

    void shutdown() override;

    void uninstall() override;

protected:
    GL_Renderer* m_renderer;
    GL_ShaderManagerDelegate* m_shaderManagerDelegate;
    GL_TextureManagerDelegate* m_textureManagerDelegate;
};

}  // namespace engine::plugin::opengl
