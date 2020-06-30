#include <Renderer/ShaderManager.hpp>

#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include <array>
#include <map>
#include <memory>

namespace engine {

namespace {

const StringView sTag("ShaderManager");

const String sRootShaderFolder("shaders");

const String sShaderDescriptorFolder("descriptor");

const std::array<ShaderType, 3> sAvailableShaderTypes = {{
    ShaderType::VERTEX,
    ShaderType::FRAGMENT,
    ShaderType::GEOMETRY,
}};

}  // namespace

template <>
ShaderManager* Singleton<ShaderManager>::sInstance = nullptr;

ShaderManager& ShaderManager::GetInstance() {
    return Singleton<ShaderManager>::GetInstance();
}

ShaderManager* ShaderManager::GetInstancePtr() {
    return Singleton<ShaderManager>::GetInstancePtr();
}

ShaderManager::ShaderManager() : m_activeShader(nullptr) {}

ShaderManager::~ShaderManager() = default;

void ShaderManager::initialize() {}

void ShaderManager::shutdown() {
    m_shaders.clear();
}

Shader* ShaderManager::loadFromFile(const String& basename) {
    Shader* newShader = getShader(basename);
    if (newShader != nullptr) {
        return newShader;
    }

    std::unique_ptr<Shader> shader = createShader();

    FileSystem& fs = FileSystem::GetInstance();

    String shaderFolder = fs.join(sRootShaderFolder, getShaderFolder());
    String shaderDescriptorFolder = fs.join(sRootShaderFolder, sShaderDescriptorFolder);

    for (auto shaderType : sAvailableShaderTypes) {
        bool ok = true;

        const char* shaderExtension = "";
        switch (shaderType) {
            case ShaderType::VERTEX:
                shaderExtension = ".vert";
                break;
            case ShaderType::FRAGMENT:
                shaderExtension = ".frag";
                break;
            case ShaderType::GEOMETRY:
                shaderExtension = ".geom";
                break;
        }

        String filename = fs.join(shaderFolder, basename + shaderExtension);

        bool filenameExist = fs.fileExists(filename);

        // Vertex and Fragment shaders are completly required
        if (!filenameExist && (shaderType == ShaderType::VERTEX || shaderType == ShaderType::FRAGMENT)) {
            LogError(sTag, "Could not find file: {}", filename);
            ok = false;
        }

        if (filenameExist) {
            Vector<byte> filenameData;
            fs.loadFileData(filename, &filenameData);
            if (!shader->loadFromMemory(filenameData.data(), filenameData.size(), shaderType)) {
                LogError(sTag, "Could not load shader: {}", basename);
                ok = false;
            }
        }

        if (!ok) {
            return nullptr;
        }
    }

    String filename = fs.join(shaderDescriptorFolder, "{}.json"_format(basename));
    Vector<byte> jsonData;
    fs.loadFileData(filename, &jsonData);
    bool isValidDescriptor = json::accept(jsonData.begin(), jsonData.end());
    if (isValidDescriptor) {
        json j = json::parse(jsonData.begin(), jsonData.end());
        shader->setDescriptor(std::move(j));
    } else {
        LogError(sTag, "Could not load shader descriptor: {}", basename);
        return nullptr;
    }

    newShader = shader.get();
    if (shader != nullptr) {
        m_shaders[basename] = std::move(shader);
    }

    if (m_activeShader == nullptr) {
        m_activeShader = newShader;
    }

    return newShader;
}

Shader* ShaderManager::loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderData) {
    if (getShader(name) != nullptr) {
        LogError(sTag, "Shader '{}' already loaded");
        return nullptr;
    }

    // Vertex and Fragment shaders are completly required
    auto itVertex = shaderData.find(ShaderType::VERTEX);
    auto itFragment = shaderData.find(ShaderType::FRAGMENT);
    if (itVertex == shaderData.end() || itFragment == shaderData.end()) {
        LogDebug(sTag, "Vertex or Fragment shader not provided");
        return nullptr;
    }

    std::unique_ptr<Shader> newShader = createShader();

    for (const auto& shaderDataPair : shaderData) {
        bool ok = true;

        ShaderType shaderType = shaderDataPair.first;
        String* shaderSource = shaderDataPair.second;

        auto shaderSourceData = shaderSource->toUtf8();
        if (!newShader->loadFromMemory(reinterpret_cast<const byte*>(shaderSourceData.data()), shaderSourceData.size(),
                                       shaderType)) {
            LogDebug(sTag, "Could not load shader: {}", static_cast<int>(shaderType));
            ok = false;
        }

        if (!ok) {
            return nullptr;
        }
    }

    m_shaders[name] = std::move(newShader);
    return m_shaders[name].get();
}

Shader* ShaderManager::getShader(const String& name) {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second.get() : nullptr;
}

void ShaderManager::setActiveShader(const String& name) {
    Shader* foundShader = getShader(name);
    if (foundShader != nullptr) {
        m_activeShader = foundShader;
        useShader(m_activeShader);
    } else {
        LogError(sTag, "Could not find a Shader named: {}", name);
    }
}

Shader* ShaderManager::getActiveShader() {
    return m_activeShader;
}

}  // namespace engine
