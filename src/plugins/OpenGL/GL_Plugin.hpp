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
    const String& getName() const;

    /// @copydoc Plugin::install
    void install();

    /// @copydoc Plugin::initialize
    void initialize();

    /// @copydoc Plugin::shutdown
    void shutdown();

    /// @copydoc Plugin::uninstall
    void uninstall();

protected:
    GL_Renderer* m_renderer;
    GL_ShaderManagerDelegate* m_shader_manager_delegate;
    GL_TextureManagerDelegate* m_texture_manager_delegate;
};

}  // namespace engine
