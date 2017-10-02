#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Texture2D.hpp>

namespace engine {

class Texture2D;
class Image;
class String;

class ENGINE_API TextureManager : public Singleton<TextureManager> {
public:
    TextureManager();
    virtual ~TextureManager();

    ////////////////////////////////////////////////////////////
    /// @brief Load a texture from the filesystem
    ///
    /// @return On success returns the Texture2D handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Texture2D* LoadFromFile(const String& basename);

    ////////////////////////////////////////////////////////////
    /// @brief Load a texture from a Image
    ///
    /// @return On success returns the Texture2D handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Texture2D* LoadFromImage(const String& name, const Image& image);

    Texture2D* GetTexture2D(const String& name);

    void SetActiveTexture2D(const String& basename);

    Texture2D* GetActiveTexture2D();

protected:
    virtual Texture2D* CreateTexture2D() = 0;

    virtual void DeleteTexture2D(Texture2D* texture) = 0;

    virtual void SetActiveTexture2D(Texture2D* texture) = 0;

    Texture2D* m_active_texture;
    std::map<String, Texture2D*> m_textures;
};

}  // engine
