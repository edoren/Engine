#include <Renderer/TextureManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

namespace engine {

namespace {

const String sTag("TextureManager");

const String sRootTextureFolder("textures");

}  // namespace

template <>
TextureManager* Singleton<TextureManager>::sInstance = nullptr;

TextureManager::TextureManager() : m_active_texture(nullptr), m_textures() {}

TextureManager::~TextureManager() {
    if (m_textures.size() > 0) {
        LogDebug(sTag, "Textures2D not deleted.");
    }
}

Texture2D* TextureManager::LoadFromFile(const String& basename) {
    FileSystem& fs = FileSystem::GetInstance();

    String filename = fs.Join(sRootTextureFolder, basename);

    bool filename_exist = fs.FileExists(filename);
    if (filename_exist) {
        Image image;
        if (!image.LoadFromFile(filename)) {
            LogDebug(sTag, "Could create Image from file: " + basename);
            return nullptr;
        }
        return LoadFromImage(basename, image);
    } else {
        LogError(sTag,
                 "Texture2D not loaded. File '{}' "
                 "not found."_format(filename.ToUtf8()));
        return nullptr;
    }
}

Texture2D* TextureManager::LoadFromImage(const String& name,
                                         const Image& image) {
    Texture2D* new_texture = GetTexture2D(name);
    if (new_texture != nullptr) {
        return new_texture;
    }

    new_texture = CreateTexture2D();
    if (new_texture != nullptr) {
        LogDebug(sTag, "Loading Texture: " + name);
        if (!new_texture->LoadFromImage(image)) {
            LogDebug(sTag, "Could not load Texture: " + name);
            DeleteTexture2D(new_texture);
            new_texture = nullptr;
        }
    } else {
        LogError(sTag, "Texture2D could not be created");
    }

    if (new_texture != nullptr) {
        m_textures[name] = new_texture;
    }
    return new_texture;
}

Texture2D* TextureManager::GetTexture2D(const String& name) {
    auto it = m_textures.find(name);
    return (it != m_textures.end()) ? it->second : nullptr;
}

void TextureManager::SetActiveTexture2D(const String& name) {
    Texture2D* found_texture = GetTexture2D(name);
    if (found_texture != nullptr) {
        m_active_texture = found_texture;
        SetActiveTexture2D(m_active_texture);
    } else {
        LogError(sTag,
                 "Could not find a Texture2D named: {}"_format(name.ToUtf8()));
    }
}

Texture2D* TextureManager::GetActiveTexture2D() {
    return m_active_texture;
}

}  // engine