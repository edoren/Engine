#include <Core/Main.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderStates.hpp>
#include <Renderer/Texture2D.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/FileSystem.hpp>
#include <System/IOStream.hpp>
#include <System/JSON.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <Util/Container/Vector.hpp>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>

#include <array>
#include <memory>
#include <utility>

namespace engine {

namespace {

const String sTag("Model");

const String sRootModelFolder("models");

class CustomAssimpIOStream : public Assimp::IOStream {
    friend class CustomAssimpIOSystem;

protected:
    // Constructor protected for private usage by CustomAssimpIOSystem
    CustomAssimpIOStream(const char* pFile, const char* pMode) {
        FileSystem& fs = FileSystem::GetInstance();
        for (const String& path : fs.getSearchPaths()) {
            String filePath = fs.join(path, pFile);
            if (m_file.open(filePath.getData(), pMode)) {
                break;
            }
        }
        if (!m_file.isOpen()) {
            LogError("CustomAssimpIOStream", String("Could not open file") + pFile);
        }
    }

public:
    ~CustomAssimpIOStream() override {
        m_file.close();
    }

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override {
        return m_file.read(pvBuffer, pSize, pCount);
    }

    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override {
        return m_file.write(pvBuffer, pSize, pCount);
    }

    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override {
        int64 ret = m_file.seek(pOffset, static_cast<engine::IOStream::Origin>(pOrigin));
        return static_cast<aiReturn>(ret);
    }

    size_t Tell() const override {
        return static_cast<size_t>(m_file.tell());
    }

    size_t FileSize() const override {
        return m_file.getSize();
    }

    void Flush() override {
        (void)0;
    }

private:
    engine::IOStream m_file;
};

class CustomAssimpIOSystem : public Assimp::IOSystem {
public:
    CustomAssimpIOSystem() = default;

    ~CustomAssimpIOSystem() override = default;

    bool Exists(const char* pFile) const override {
        return FileSystem::GetInstance().fileExists(pFile);
    }

    char getOsSeparator() const override {
        return FileSystem::GetInstance().getOsSeparator();
    }

    Assimp::IOStream* Open(const char* pFile, const char* pMode) override {
        return new CustomAssimpIOStream(pFile, pMode);
    }

    void Close(Assimp::IOStream* pFile) override {
        delete pFile;
    }
};

TextureType GetTextureTypeFromString(const String& name) {
    if (name == "diffuse") {
        return TextureType::DIFFUSE;
    }
    if (name == "specular") {
        return TextureType::SPECULAR;
    }
    if (name == "normals") {
        return TextureType::NORMALS;
    }
    if (name == "lightmap") {
        return TextureType::LIGHTMAP;
    }
    if (name == "emissive") {
        return TextureType::EMISSIVE;
    }
    return TextureType::UNKNOWN;
}

TextureType GetTextureTypeFromAiTextureType(aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE:
            return TextureType::DIFFUSE;

        case aiTextureType_SPECULAR:
            return TextureType::SPECULAR;

        case aiTextureType_NORMALS:
            return TextureType::NORMALS;

        case aiTextureType_LIGHTMAP:
            return TextureType::LIGHTMAP;

        case aiTextureType_EMISSIVE:
            return TextureType::EMISSIVE;

        default:
            return TextureType::UNKNOWN;
    }
}

}  // namespace

Model::Model() = default;

Model::~Model() {
    for (auto& mesh : m_meshes) {
        mesh.reset();
    }
}

void Model::setTransform(const Transform& transform) {
    m_transform = transform;
}

void Model::draw(RenderWindow& target, const RenderStates& states) const {
    for (const auto& mesh : m_meshes) {
        mesh->draw(target, states);
    }
}

void Model::loadModel(const String& path) {
    Assimp::Importer importer;

    FileSystem& fs = FileSystem::GetInstance();
    String filename = fs.join(sRootModelFolder, path);

    String pathNoext = path.subString(0, path.findLastOf("."));
    String jsonFilename = fs.join(sRootModelFolder, pathNoext + ".json");
    if (fs.fileExists(jsonFilename)) {
        Vector<byte> jsonData;

        fs.loadFileData(jsonFilename, &jsonData);
        if (json::accept(jsonData.begin(), jsonData.end())) {
            m_descriptor = json::parse(jsonData.begin(), jsonData.end());
            LogDebug(sTag, "Loading descriptor: " + jsonFilename);
        }
    }

    const json& properties = m_descriptor["properties"];
    if (!m_descriptor.is_null() && !properties.is_null()) {
        Transform modelMatrix;
        const json& scale = properties["scale"];
        const json& rotation = properties["rotation"];
        if (!scale.is_null()) {
            modelMatrix.scale(math::vec3(float(scale)));
        }
        if (!rotation.is_null()) {
            modelMatrix.rotate({float(rotation[0]), float(rotation[1]), float(rotation[2])});
        }
        m_transform = modelMatrix;
    }

    importer.SetIOHandler(new CustomAssimpIOSystem());

    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LogError("Model", String("ERROR::ASSIMP::") + importer.GetErrorString());
        return;
    }

    m_relativeDirectory = path.subString(0, path.findLastOf("/\\"));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }
    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    Vector<Vertex> vertices;
    Vector<uint32> indices;
    Vector<std::pair<Texture2D*, TextureType>> textures;

    // Process vertices
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        math::vec3 vector;

        // Swap X and Z axis by rotating the model matrix 90 degrees on Y
        math::mat4 rotationMatrix = math::RotateAxisY(math::Radians(90.0F));

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        math::vec4 resultPosition = rotationMatrix * math::vec4(vector, 1);
        vertex.position = resultPosition.xyz();

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            math::vec4 resultNormal = rotationMatrix * math::vec4(vector, 0.0);
            vertex.normal = resultNormal.xyz();
        }

        // Does the mesh contain texture coordinates
        if (mesh->HasTextureCoords(0)) {
            math::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }

        // Process vertex positions, normals and texture coordinates
        vertices.push_back(vertex);
    }

    // Process indices
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) {
            continue;
        }
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    // Process material
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::array<aiTextureType, 2> enabledTextureTypes = {
        {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
        },
    };

    FileSystem& fs = FileSystem::GetInstance();

    Vector<std::pair<TextureType, String>> textureFilenames;

    auto loadTexturesFromMaterial = [&textureFilenames, &fs, this](const json& jsonMaterial) {
        const json& jsonTextures = jsonMaterial["textures"];
        for (const json& jsonTexture : jsonTextures) {
            const json& type = jsonTexture["type"];
            const json& name = jsonTexture["name"];
            if (type.is_string() && name.is_string()) {
                textureFilenames.emplace_back(GetTextureTypeFromString(type), fs.join(m_relativeDirectory, name));
            }
        }
    };

    const json& jsonMaterials = m_descriptor["materials"];
    size_t materialsCount = jsonMaterials.size();
    if (!m_descriptor.is_null() && !jsonMaterials.is_null()) {
        int32 materialId = -1;

        for (const json& jsonMesh : m_descriptor["meshes"]) {
            const json& name = jsonMesh["name"];
            if (name.is_string() && name == mesh->mName.C_Str()) {
                materialId = jsonMesh["material_id"];
                break;
            }
        }

        if (materialsCount == 1 && materialId < 0) {
            loadTexturesFromMaterial(jsonMaterials[0]);
        } else {
            for (const json& jsonMaterial : jsonMaterials) {
                if (jsonMaterial["id"] == materialId) {
                    loadTexturesFromMaterial(jsonMaterial);
                }
            }
        }
    } else {
        for (aiTextureType type : enabledTextureTypes) {
            unsigned int textureCount = material->GetTextureCount(type);
            for (unsigned int i = 0; i < textureCount; i++) {
                aiString str;
                material->GetTexture(type, i, &str);

                textureFilenames.emplace_back(
                    std::make_pair(GetTextureTypeFromAiTextureType(type), fs.join(m_relativeDirectory, str.C_Str())));
            }
        }
    }

    TextureManager& textureManager = TextureManager::GetInstance();

    for (auto& pair : textureFilenames) {
        TextureType type = pair.first;
        const String& filename = pair.second;
        Texture2D* texture = textureManager.loadFromFile(filename);
        textures.emplace_back(texture, type);
    }

    if (textures.empty()) {
        Texture2D* texture = textureManager.getTexture2D(TextureManager::sDefaultTextureId);
        textures.emplace_back(texture, TextureType::DIFFUSE);
    }

    ModelManager& modelManager = ModelManager::GetInstance();
    std::unique_ptr<Mesh> ret = modelManager.createMesh();
    ret->loadFromData(vertices, indices, textures);

    return ret;
}

}  // namespace engine
