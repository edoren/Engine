#include <Core/Main.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

namespace engine {

namespace {

const String sTag("TextureManager");

const String sRootTextureFolder("textures");

}  // namespace

const String TextureManager::sDefaultTextureId("DEFAULT");

template <>
TextureManager* Singleton<TextureManager>::sInstance = nullptr;

TextureManager& TextureManager::GetInstance() {
    return Singleton<TextureManager>::GetInstance();
}

TextureManager* TextureManager::GetInstancePtr() {
    return Singleton<TextureManager>::GetInstancePtr();
}

TextureManager::TextureManager() : m_activeTexture(nullptr) {}

TextureManager::~TextureManager() {}

void TextureManager::initialize() {
    math::uvec2 defaultTextureSize(300, 300);

    std::vector<Color32> defaultTextureData(defaultTextureSize.x * defaultTextureSize.y, Color32::sGray);

    Image defaultImage;
    defaultImage.loadFromMemory(defaultTextureData.data(), defaultTextureSize.x, defaultTextureSize.y);

    loadFromImage(sDefaultTextureId, defaultImage);
}

void TextureManager::shutdown() {
    m_textures.clear();
}

Texture2D* TextureManager::loadFromFile(const String& basename) {
    FileSystem& fs = FileSystem::GetInstance();

    String filename = fs.join(sRootTextureFolder, basename);

    bool filename_exist = fs.fileExists(filename);
    if (filename_exist) {
        Image image;
        if (!image.loadFromFile(filename)) {
            LogDebug(sTag, "Could create Image from file: " + basename);
            return nullptr;
        }
        return loadFromImage(basename, image);
    }
    LogError(sTag,
             "Texture2D not loaded. File '{}' "
             "not found."_format(filename.toUtf8()));
    return nullptr;
}

Texture2D* TextureManager::loadFromImage(const String& name, const Image& image) {
    Texture2D* texture = getTexture2D(name);
    if (texture != nullptr) {
        return texture;
    }

    std::unique_ptr<Texture2D> new_texture = createTexture2D();
    if (new_texture != nullptr) {
        LogDebug(sTag, "Loading Texture: " + name);
        if (!new_texture->loadFromImage(image)) {
            LogDebug(sTag, "Could not load Texture: " + name);
            new_texture.reset();
        }
    } else {
        LogError(sTag, "Texture2D could not be created");
    }

    texture = new_texture.get();
    if (new_texture != nullptr) {
        m_textures[name] = std::move(new_texture);
    }

    // TMP
    if (m_activeTexture == nullptr) {
        m_activeTexture = texture;
    }

    return texture;
}

Texture2D* TextureManager::getTexture2D(const String& name) {
    auto it = m_textures.find(name);
    return (it != m_textures.end()) ? it->second.get() : nullptr;
}

void TextureManager::setActiveTexture2D(const String& name) {
    Texture2D* found_texture = getTexture2D(name);
    if (found_texture != nullptr) {
        m_activeTexture = found_texture;
        useTexture2D(m_activeTexture);
    } else {
        LogError(sTag, "Could not find a Texture2D named: {}"_format(name.toUtf8()));
    }
}

Texture2D* TextureManager::getActiveTexture2D() {
    return m_activeTexture;
}

}  // namespace engine
