#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Texture2D.hpp>

namespace engine {

class Texture2D;
class Image;
class String;

class ENGINE_API TextureManager : public Singleton<TextureManager> {
public:
    static const String DEFAULT_TEXTURE_ID;

public:
    TextureManager();

    virtual ~TextureManager();

    virtual void Initialize();

    virtual void Shutdown();

    ////////////////////////////////////////////////////////////
    /// @brief Load a texture from the filesystem
    ///
    /// @return On success returns the Texture2D handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    virtual Texture2D* LoadFromFile(const String& basename);

    ////////////////////////////////////////////////////////////
    /// @brief Load a texture from a Image
    ///
    /// @return On success returns the Texture2D handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    virtual Texture2D* LoadFromImage(const String& name, const Image& image);

    Texture2D* GetTexture2D(const String& name);

    void SetActiveTexture2D(const String& basename);

    Texture2D* GetActiveTexture2D();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static TextureManager& GetInstance();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static TextureManager* GetInstancePtr();

protected:
    virtual std::unique_ptr<Texture2D> CreateTexture2D() = 0;
    virtual void UseTexture2D(Texture2D* texture) = 0;

    Texture2D* m_active_texture;
    std::map<String, std::unique_ptr<Texture2D>> m_textures;
};

}  // namespace engine
