#pragma once

#include <Renderer/TextureManager.hpp>

#include "GL_Config.hpp"
#include "GL_Dependencies.hpp"

#include <memory>

namespace engine::plugin::opengl {

class OPENGL_PLUGIN_API GL_TextureManager : public TextureManager {
public:
    GL_TextureManager();

    ~GL_TextureManager() override;

    /**
     * @copydoc TextureManager::GetInstance
     */
    static GL_TextureManager& GetInstance();

    /**
     * @copydoc TextureManager::GetInstance
     */
    static GL_TextureManager* GetInstancePtr();

protected:
    std::unique_ptr<Texture2D> createTexture2D() override;
    void useTexture2D(Texture2D* texture) override;
};

}  // namespace engine::plugin::opengl
