#pragma once

#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Texture2D.hpp>
#include <Util/Precompiled.hpp>
#include <System/FileSystem.hpp>

namespace engine {

class ResourceManager : NonCopyable {
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(int argc, char* argv[]);

    Shader* FindShader(const String& basename);
    Texture2D* FindTexture2D(const String& basename);

    Shader* LoadShader(const String& basename);
    Texture2D* LoadTexture2D(const String& basename);

private:
    filesystem::Path basedir_;
    std::map<String, Shader> shader_map_;
    std::map<String, Texture2D> texture_2d_map_;
    // std::map<String, Material *> material_map_;
    // std::map<String, Mesh *> mesh_map_;
};

}  // namespace engine
