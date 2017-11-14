#include <Core/Main.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Renderer/Texture2D.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/FileSystem.hpp>
#include <System/IOStream.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>

namespace engine {

namespace {

static const String sRootModelFolder("models");

class CustomAssimpIOStream : public Assimp::IOStream {
    friend class CustomAssimpIOSystem;

protected:
    // Constructor protected for private usage by CustomAssimpIOSystem
    CustomAssimpIOStream(const char* pFile, const char* pMode) {
        FileSystem& fs = FileSystem::GetInstance();
        for (const String& path : fs.GetSearchPaths()) {
            String file_path = fs.Join(path, pFile);
            if (m_file.Open(file_path.GetData(), pMode)) break;
        }
        if (!m_file.IsOpen()) {
            LogError("CustomAssimpIOStream",
                     String("Could not open file") + pFile);
        }
    }

public:
    ~CustomAssimpIOStream() {
        m_file.Close();
    }

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override {
        return m_file.Read(pvBuffer, pSize, pCount);
    }

    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override {
        return m_file.Write(pvBuffer, pSize, pCount);
    }

    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override {
        int64 ret = m_file.Seek(pOffset,
                                static_cast<engine::IOStream::Origin>(pOrigin));
        return static_cast<aiReturn>(ret);
    }

    size_t Tell() const override {
        return static_cast<size_t>(m_file.Tell());
    }

    size_t FileSize() const override {
        return m_file.GetSize();
    }

    void Flush() override {
        (void)0;
    }

private:
    engine::IOStream m_file;
};

class CustomAssimpIOSystem : public Assimp::IOSystem {
public:
    CustomAssimpIOSystem() {}

    ~CustomAssimpIOSystem() {}

    bool Exists(const char* pFile) const override {
        return FileSystem::GetInstance().FileExists(pFile);
    }

    char getOsSeparator() const override {
        return FileSystem::GetInstance().GetOsSeparator();
    }

    Assimp::IOStream* Open(const char* pFile, const char* pMode) override {
        return new CustomAssimpIOStream(pFile, pMode);
    }

    void Close(Assimp::IOStream* pFile) override {
        delete pFile;
    }
};

}  // namespace

Model::Model(const String& path) {
    LoadModel(path);
}

Model::~Model() {
    for (auto mesh : m_meshes) {
        delete mesh;
    }
}

void Model::Draw(RenderWindow& target) const {
    ENGINE_UNUSED(target);
    for (size_t i = 0; i < m_meshes.size(); i++) {
        m_meshes[i]->Draw();
    }
}

void Model::LoadModel(const String& path) {
    Assimp::Importer importer;

    FileSystem& fs = FileSystem::GetInstance();
    String filename = fs.Join(sRootModelFolder, path);

    importer.SetIOHandler(new CustomAssimpIOSystem());

    const aiScene* scene =
        importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        LogError("Model",
                 String("ERROR::ASSIMP::") + importer.GetErrorString());
        return;
    }

    m_relative_directory = path.SubString(0, path.FindLastOf("/\\"));

    ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(mesh, scene));
    }
    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::vector<std::pair<Texture2D*, TextureType>> textures;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        math::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        // Does the mesh contain texture coordinates
        if (mesh->HasTextureCoords(0)) {
            math::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = vec;
        }

        // Process vertex positions, normals and texture coordinates
        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(static_cast<uint32>(face.mIndices[j]));
        }
    }

    // Process material
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::array<aiTextureType, 2> enabled_texture_types = {
        {aiTextureType_DIFFUSE, aiTextureType_SPECULAR},
    };

    TextureManager& texture_manager = TextureManager::GetInstance();
    FileSystem& fs = FileSystem::GetInstance();
    for (aiTextureType type : enabled_texture_types) {
        unsigned int texture_count = material->GetTextureCount(type);
        for (unsigned int i = 0; i < texture_count; i++) {
            aiString str;
            material->GetTexture(type, i, &str);

            String name = fs.Join(m_relative_directory, str.C_Str());

            Texture2D* texture = texture_manager.LoadFromFile(name);
            TextureType texture_type = TextureType::eNone;

            switch (type) {
                case aiTextureType_DIFFUSE:
                    texture_type = TextureType::eDiffuse;
                    break;
                case aiTextureType_SPECULAR:
                    texture_type = TextureType::eSpecular;
                    break;
                case aiTextureType_NORMALS:
                    texture_type = TextureType::eNormals;
                    break;
                default:
                    texture_type = TextureType::eUnknown;
                    break;
            }

            textures.push_back(std::make_pair(texture, texture_type));
        }
    }

    RendererFactory& factory = Main::GetInstance().GetActiveRendererFactory();
    Mesh* ret = factory.CreateMesh();
    ret->LoadFromData(vertices, indices, textures);

    return ret;
}

}  // namespace engine
