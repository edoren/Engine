#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Renderer/Transform.hpp>
#include <System/JSON.hpp>
#include <System/String.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/NonCopyable.hpp>

#include <memory>

struct aiMesh;
struct aiNode;
struct aiScene;

namespace engine {

class Texture2D;

class ENGINE_API Model : NonCopyable {
    friend class ModelManager;

public:
    Model();

    virtual ~Model();

    void setTransform(const Transform& transform);

    virtual void draw(RenderWindow& target, const RenderStates& states) const;

private:
    void loadModel(const String& path);

    void processNode(aiNode* node, const aiScene* scene);

    std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);

    Vector<std::unique_ptr<Mesh>> m_meshes;
    String m_relativeDirectory;

    Transform m_transform;

    json m_descriptor;
};

}  // namespace engine
