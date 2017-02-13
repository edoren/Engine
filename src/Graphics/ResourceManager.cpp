#include <Graphics/ResourceManager.hpp>
#include <System/IO/FileLoader.hpp>
#include <System/LogManager.hpp>

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
T* FindInMap(std::map<String, T>& map, const String& name) {
    auto it = map.find(name);
    return it != map.end() ? &(it->second) : nullptr;
}

ResourceManager::ResourceManager(const filesystem::Path& resource_dir) {
    basedir_ = resource_dir;
}

ResourceManager::~ResourceManager() {}

bool ResourceManager::Initialize() {
    return true;
}

void ResourceManager::ShutDown() {}

Shader* ResourceManager::FindShader(const String& basename) {
    return FindInMap(shader_map_, basename);
}

Texture2D* ResourceManager::FindTexture2D(const String& basename) {
    return FindInMap(texture_2d_map_, basename);
}

Shader* ResourceManager::LoadShader(const String& basename) {
    auto shader = FindShader(basename);
    if (shader) return shader;
    String vs_file, ps_file;
    String filename = String(basename) + ".vert";
    if (io::FileLoader::LoadFile(filename, &vs_file)) {
        filename = String(basename) + ".frag";
        if (io::FileLoader::LoadFile(filename, &ps_file)) {
            Shader shader;
            int status;
            status = shader.LoadFromMemory(vs_file, ps_file);
            if (status) {
                shader_map_.emplace(
                    basename, std::move(shader));  // TODO: check insertion
            } else {
                LogError("ResourceManager", "Shader Error");
            }
            return FindShader(basename);
        }
    }
    LogError("ResourceManager", "Can\'t load shader: " + filename);
    // renderer_.last_error() = "Couldn\'t load: " + filename;
    return nullptr;
}

Texture2D* ResourceManager::LoadTexture2D(const String& basename) {
    Image img;
    filesystem::Path filepath = filesystem::Absolute(basename, basedir_);
    if (img.LoadFromFile(filepath.Str())) {
        LogInfo("ResourceManager", "Loading Texture: " + filepath.Str());
        Texture2D texture;
        texture.LoadFromImage(img);
        texture_2d_map_.emplace(basename,
                                std::move(texture));  // TODO: check insertion
    } else {
        LogError("ResourceManager",
                 "Could not load Texture: " + filepath.Str());
    }
    return FindTexture2D(basename);
}

}  // namespace engine
