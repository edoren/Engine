#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Texture2D.hpp>
#include <Util/Singleton.hpp>

#include <map>
#include <memory>

namespace engine {

class Texture2D;
class Image;
class String;

class ENGINE_API TextureManager : public Singleton<TextureManager> {
public:
    static const String sDefaultTextureId;

    TextureManager();

    virtual ~TextureManager();

    virtual void initialize();

    virtual void shutdown();

    /**
     * @brief Load a texture from the filesystem
     *
     * @return On success returns the Texture2D handler or nullptr on failure
     */
    virtual Texture2D* loadFromFile(const String& basename);

    /**
     * @brief Load a texture from a Image
     *
     * @return On success returns the Texture2D handler or nullptr on failure
     */
    virtual Texture2D* loadFromImage(const String& name, const Image& image);

    Texture2D* getTexture2D(const String& name);

    void setActiveTexture2D(const String& basename);

    Texture2D* getActiveTexture2D();

    /**
     * @copydoc Main::GetInstance
     */
    static TextureManager& GetInstance();

    /**
     * @copydoc Main::GetInstance
     */
    static TextureManager* GetInstancePtr();

protected:
    virtual std::unique_ptr<Texture2D> createTexture2D() = 0;
    virtual void useTexture2D(Texture2D* texture) = 0;

    Texture2D* m_activeTexture;
    std::map<String, std::unique_ptr<Texture2D>> m_textures;
};

}  // namespace engine
