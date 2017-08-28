#pragma once

#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Texture2D.hpp>
#include <System/FileSystem.hpp>
#include <Util/Prerequisites.hpp>

namespace engine {

class ENGINE_API ResourceManager : public Singleton<ResourceManager> {
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize();

    void Shutdown();

    Shader* FindShader(const String& basename);
    Texture2D* FindTexture2D(const String& basename);

    Shader* LoadShader(const String& basename, const String& vertex_file,
                       const String& fragment_file);
    Texture2D* LoadTexture2D(const String& basename);

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
    static ResourceManager& GetInstance();

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
    static ResourceManager* GetInstancePtr();

private:
    std::map<String, Shader*> m_shader_map;
    std::map<String, Texture2D*> m_texture_2d_map;
    // std::map<String, Material *> m_material_map;
    // std::map<String, Mesh *> m_mesh_map;
};

}  // namespace engine
