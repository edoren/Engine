#include <Renderer/ShaderManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

namespace engine {

namespace {

const String sTag("ShaderManager");

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
    Shader* new_shader = getShader(basename);
    if (new_shader != nullptr) {
        return new_shader;
    }

    std::unique_ptr<Shader> shader = createShader();

    FileSystem& fs = FileSystem::GetInstance();

    String shader_folder = fs.join(sRootShaderFolder, getShaderFolder());
    String shader_descriptor_folder = fs.join(sRootShaderFolder, sShaderDescriptorFolder);

    for (auto shader_type : sAvailableShaderTypes) {
        bool ok = true;

        const char* shader_extension = "";
        switch (shader_type) {
            case ShaderType::VERTEX:
                shader_extension = ".vert";
                break;
            case ShaderType::FRAGMENT:
                shader_extension = ".frag";
                break;
            case ShaderType::GEOMETRY:
                shader_extension = ".geom";
                break;
        }

        String filename = fs.join(shader_folder, basename + shader_extension);

        bool filename_exist = fs.fileExists(filename);

        // Vertex and Fragment shaders are completly required
        if (!filename_exist && (shader_type == ShaderType::VERTEX || shader_type == ShaderType::FRAGMENT)) {
            LogError(sTag, "Could not find file: {}"_format(filename));
            ok = false;
        }

        if (filename_exist) {
            std::vector<byte> filename_data;
            fs.loadFileData(filename, &filename_data);
            if (!shader->loadFromMemory(filename_data.data(), filename_data.size(), shader_type)) {
                LogError(sTag, "Could not load shader: {}"_format(basename));
                ok = false;
            }
        }

        if (!ok) {
            return nullptr;
        }
    }

    String filename = fs.join(shader_descriptor_folder, basename + ".json");
    std::vector<byte> json_data;
    fs.loadFileData(filename, &json_data);
    bool is_valid_descriptor = json::accept(json_data.begin(), json_data.end());
    if (is_valid_descriptor) {
        json j = json::parse(json_data.begin(), json_data.end());
        shader->setDescriptor(std::move(j));
    } else {
        LogError(sTag, "Could not load shader descriptor: {}"_format(basename));
        return nullptr;
    }

    new_shader = shader.get();
    if (shader != nullptr) {
        m_shaders[basename] = std::move(shader);
    }

    if (m_activeShader == nullptr) {
        m_activeShader = new_shader;
    }

    return new_shader;
}

Shader* ShaderManager::loadFromMemory(const String& name, std::map<ShaderType, String*> shader_data) {
    if (getShader(name) != nullptr) {
        LogError(sTag, "Shader '{}' already loaded");
        return nullptr;
    }

    // Vertex and Fragment shaders are completly required
    auto it_vertex = shader_data.find(ShaderType::VERTEX);
    auto it_fragment = shader_data.find(ShaderType::FRAGMENT);
    if (it_vertex == shader_data.end() || it_fragment == shader_data.end()) {
        LogDebug(sTag, "Vertex or Fragment shader not provided");
        return nullptr;
    }

    std::unique_ptr<Shader> new_shader = createShader();

    for (auto& shader_data_pair : shader_data) {
        bool ok = true;

        ShaderType shader_type = shader_data_pair.first;
        String* shader_source = shader_data_pair.second;

        auto shader_source_data = shader_source->toUtf8();
        if (!new_shader->loadFromMemory(reinterpret_cast<const byte*>(shader_source_data.data()),
                                        shader_source_data.size(), shader_type)) {
            LogDebug(sTag, "Could not load shader: {}"_format(static_cast<int>(shader_type)));
            ok = false;
        }

        if (!ok) {
            return nullptr;
        }
    }

    m_shaders[name] = std::move(new_shader);
    return m_shaders[name].get();
}

Shader* ShaderManager::getShader(const String& name) {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second.get() : nullptr;
}

void ShaderManager::setActiveShader(const String& name) {
    Shader* found_shader = getShader(name);
    if (found_shader != nullptr) {
        m_activeShader = found_shader;
        useShader(m_activeShader);
    } else {
        LogError(sTag, "Could not find a Shader named: {}"_format(name));
    }
}

Shader* ShaderManager::getActiveShader() {
    return m_activeShader;
}

}  // namespace engine
