#pragma once

#include <Core/Plugin.hpp>

#include "GL_Config.hpp"

namespace engine {

class String;

class GL_Renderer;
class GL_ShaderManagerDelegate;
class GL_TextureManagerDelegate;

class OPENGL_PLUGIN_API GL_Plugin : public Plugin {
public:
    GL_Plugin();

    /// @copydoc Plugin::getName
    const String& getName() const override;

    /// @copydoc Plugin::install
    void install() override;

    /// @copydoc Plugin::initialize
    void initialize() override;

    /// @copydoc Plugin::shutdown
    void shutdown() override;

    /// @copydoc Plugin::uninstall
    void uninstall() override;

protected:
    GL_Renderer* m_renderer;
    GL_ShaderManagerDelegate* m_shaderManagerDelegate;
    GL_TextureManagerDelegate* m_textureManagerDelegate;
};

}  // namespace engine
