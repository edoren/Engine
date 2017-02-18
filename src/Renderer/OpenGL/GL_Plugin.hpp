#pragma once

#include <Core/Plugin.hpp>
#include <System/String.hpp>

#include "GL_Renderer.hpp"

namespace engine {

class GL_Plugin : public Plugin {
public:
    GL_Plugin();

    /// @copydoc Plugin::GetName
    const String& GetName() const;

    /// @copydoc Plugin::Install
    void Install();

    /// @copydoc Plugin::Initialize
    void Initialize();

    /// @copydoc Plugin::Shutdown
    void Shutdown();

    /// @copydoc Plugin::Uninstall
    void Uninstall();

protected:
    GL_Renderer* m_renderer;
};

}  // namespace engine
