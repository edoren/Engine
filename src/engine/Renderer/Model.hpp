#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Renderer/Transform.hpp>
#include <System/JSON.hpp>
#include <System/String.hpp>

struct aiMesh;
struct aiNode;
struct aiScene;

namespace engine {

class Texture2D;

class ENGINE_API Model {
    friend class ModelManager;

public:
    Model();
    virtual ~Model();

    void SetTransform(const Transform& transform);

    virtual void Draw(RenderWindow& target, const RenderStates& states) const;

private:
    void LoadModel(const String& path);

    void ProcessNode(aiNode* node, const aiScene* scene);

    std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    String m_relative_directory;

    Transform m_transform;

    json m_descriptor;
};

}  // namespace engine
