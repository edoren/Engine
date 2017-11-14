#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>

namespace engine {

class Mesh;

class ENGINE_API MeshManager : public Singleton<MeshManager> {
public:
    MeshManager();
    virtual ~MeshManager();

    ////////////////////////////////////////////////////////////
    /// @brief Load a texture from the filesystem
    ///
    /// @return On success returns the Mesh handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Mesh* LoadFromFile(const String& basename);

    ////////////////////////////////////////////////////////////
    /// @brief Load a mesh from a vector of vertices, indices and textures
    ///
    /// @return On success returns the Mesh handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Mesh* LoadFromData(
        std::vector<Vertex> vertices, std::vector<uint32> indices,
        std::vector<std::pair<Texture2D*, TextureType>> textures);

    Mesh* GetMesh(const String& name);

    void SetActiveMesh(const String& basename);

    Mesh* GetActiveMesh();

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
    static MeshManager& GetInstance();

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
    static MeshManager* GetInstancePtr();

protected:
    virtual Mesh* CreateMesh() = 0;

    virtual void DeleteMesh(Mesh* texture) = 0;

    virtual void UseMesh(Mesh* texture) = 0;

    Mesh* m_active_texture;
    std::map<String, Mesh*> m_textures;
};

}  // engine
