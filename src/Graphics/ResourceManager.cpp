#include <Core/Main.hpp>
#include <Graphics/ResourceManager.hpp>
#include <Renderer/RendererFactory.hpp>
#include <System/LogManager.hpp>

#include <SDL.h>

namespace engine {

namespace {

const String sTag("ResourceManager");

}  // namespace

template <>
ResourceManager* Singleton<ResourceManager>::sInstance = nullptr;

ResourceManager& ResourceManager::GetInstance() {
    assert(sInstance);
    return (*sInstance);
}

ResourceManager* ResourceManager::GetInstancePtr() {
    return sInstance;
}

template <typename T>
T FindInMap(std::map<String, T>& map, const String& name) {
    auto it = map.find(name);
    return it != map.end() ? it->second : nullptr;
}

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {
    Shutdown();
}

bool ResourceManager::Initialize() {
    return true;
}

void ResourceManager::Shutdown() {
    for (auto& it : m_shader_map) {
        delete it.second;
    }
    m_shader_map.clear();
    for (auto& it : m_texture_2d_map) {
        delete it.second;
    }
    m_texture_2d_map.clear();
}

Shader* ResourceManager::FindShader(const String& basename) {
    return FindInMap(m_shader_map, basename);
}

Texture2D* ResourceManager::FindTexture2D(const String& basename) {
    return FindInMap(m_texture_2d_map, basename);
}

Shader* ResourceManager::LoadShader(const String& basename,
                                    const String& vertex_file,
                                    const String& fragment_file) {
    Shader* shader = nullptr;

    shader = FindShader(basename);
    if (shader) return shader;

    RendererFactory& factory = Main::GetInstance().GetActiveRendererFactory();

    shader = factory.CreateShader();

    const std::string& vertex_src = vertex_file.ToUtf8();
    const std::string& fragment_src = fragment_file.ToUtf8();

    bool vertex_status =
        shader->LoadFromMemory(reinterpret_cast<const byte*>(vertex_src.data()),
                               vertex_src.size(), ShaderType::eVertex);
    bool fragment_status = shader->LoadFromMemory(
        reinterpret_cast<const byte*>(fragment_src.data()), fragment_src.size(),
        ShaderType::eFragment);

    bool link_status = shader->Link();

    if (vertex_status && fragment_status && link_status) {
        m_shader_map[basename] = shader;
    } else {
        LogError(sTag, "Error loading Shader");
        delete shader;
        shader = nullptr;
    }

    return shader;
}

Texture2D* ResourceManager::LoadTexture2D(const String& basename) {
    Texture2D* texture = nullptr;

    texture = FindTexture2D(basename);
    if (texture) return texture;

    RendererFactory& factory = Main::GetInstance().GetActiveRendererFactory();

    Image img;
    if (img.LoadFromFile(basename)) {
        LogDebug(sTag, "Loading Texture: " + basename);
        texture = factory.CreateTexture2D();
        texture->LoadFromImage(img);
        m_texture_2d_map[basename] = texture;
    } else {
        LogError(sTag, "Could not load Texture: " + basename);
    }

    return texture;
}

}  // namespace engine
