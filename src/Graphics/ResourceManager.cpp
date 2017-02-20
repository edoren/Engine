#include <Core/Main.hpp>
#include <Graphics/ResourceManager.hpp>
#include <System/IO/FileLoader.hpp>
#include <System/LogManager.hpp>

#include <SDL.h>

namespace engine {

template <>
ResourceManager* Singleton<ResourceManager>::s_instance = nullptr;

ResourceManager& ResourceManager::GetInstance() {
    assert(s_instance);
    return (*s_instance);
}

ResourceManager* ResourceManager::GetInstancePtr() {
    return s_instance;
}

template <typename T>
T FindInMap(std::map<String, T>& map, const String& name) {
    auto it = map.find(name);
    return it != map.end() ? it->second : nullptr;
}

ResourceManager::ResourceManager() {
    char* path = SDL_GetBasePath();
    if (path) {
        m_basedir = path;
        SDL_free(path);
    }
}

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

    shader = Main::GetInstance().GetActiveRenderer().CreateShader();
    int status = shader->LoadFromMemory(vertex_file, fragment_file);
    if (status && shader) {
        m_shader_map[basename] = shader;
    } else {
        LogError("ResourceManager", "Error loading Shader");
        delete shader;
        shader = nullptr;
    }

    return shader;

    // String vs_file, ps_file;
    // String filename = String(basename) + ".vert";
    // if (io::FileLoader::LoadFile(filename, &vs_file)) {
    //     filename = String(basename) + ".frag";
    //     if (io::FileLoader::LoadFile(filename, &ps_file)) {
    //     }
    // }
    // LogError("ResourceManager", "Can't load shader: " + filename);
    // m_renderer.last_error() = "Couldn't load: " + filename;
    return nullptr;
}

Texture2D* ResourceManager::LoadTexture2D(const String& basename) {
    Image img;
    filesystem::Path filepath = filesystem::Absolute(basename, m_basedir);
    if (img.LoadFromFile(filepath.Str())) {
        LogDebug("ResourceManager", "Loading Texture: " + filepath.Str());
        Texture2D* texture =
            Main::GetInstance().GetActiveRenderer().CreateTexture2D();
        texture->LoadFromImage(img);
        m_texture_2d_map[basename] = texture;
    } else {
        LogError("ResourceManager",
                 "Could not load Texture: " + filepath.Str());
    }
    return FindTexture2D(basename);
}

}  // namespace engine
