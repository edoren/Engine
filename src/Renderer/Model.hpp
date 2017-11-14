#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Drawable.hpp>
#include <Renderer/Mesh.hpp>
#include <System/String.hpp>

struct aiMaterial;
struct aiMesh;
struct aiNode;
struct aiScene;

namespace engine {

class Texture2D;

class ENGINE_API Model : public Drawable {
public:
    Model(const String& path);

    ~Model();

protected:
    void Draw(RenderWindow& target) const override;

private:
    void LoadModel(const String& path);

    void ProcessNode(aiNode* node, const aiScene* scene);

    Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Mesh*> m_meshes;
    String m_relative_directory;

    math::mat4 m_model_matrix;
};

}  // namespace engine
