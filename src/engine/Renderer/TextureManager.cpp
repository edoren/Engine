#include <Renderer/TextureManager.hpp>

#include <Core/Main.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include <memory>

namespace engine {

namespace {

const StringView sTag("TextureManager");

const StringView sRootTextureFolder("textures");

}  // namespace

const StringView TextureManager::sDefaultTextureId("DEFAULT");

TextureManager::TextureManager() : m_activeTexture(nullptr) {}

TextureManager::~TextureManager() = default;

void TextureManager::initialize() {
    math::uvec2 defaultTextureSize(300, 300);

    Vector<Color32> defaultTextureData(defaultTextureSize.x * defaultTextureSize.y, Color32::sGray);

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

    bool filenameExist = fs.fileExists(filename);
    if (filenameExist) {
        Image image;
        if (!image.loadFromFile(filename)) {
            LogDebug(sTag, "Could create Image from file: {}", basename);
            return nullptr;
        }
        return loadFromImage(basename, image);
    }
    LogError(sTag, "Texture2D not loaded. File '{}' not found.", filename.toUtf8());
    return nullptr;
}

Texture2D* TextureManager::loadFromImage(const String& name, const Image& image) {
    Texture2D* texture = getTexture2D(name);
    if (texture != nullptr) {
        return texture;
    }

    std::unique_ptr<Texture2D> newTexture = createTexture2D();
    if (newTexture != nullptr) {
        LogDebug(sTag, "Loading Texture: {}", name);
        if (!newTexture->loadFromImage(image)) {
            LogDebug(sTag, "Could not load Texture: {}", name);
            newTexture.reset();
        }
    } else {
        LogError(sTag, "Texture2D could not be created");
    }

    texture = newTexture.get();
    if (newTexture != nullptr) {
        m_textures[name] = std::move(newTexture);
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

void TextureManager::setActiveTexture2D(const String& basename) {
    Texture2D* foundTexture = getTexture2D(basename);
    if (foundTexture != nullptr) {
        m_activeTexture = foundTexture;
        useTexture2D(m_activeTexture);
    } else {
        LogError(sTag, "Could not find a Texture2D named: {}", basename.toUtf8());
    }
}

Texture2D* TextureManager::getActiveTexture2D() {
    return m_activeTexture;
}

}  // namespace engine
