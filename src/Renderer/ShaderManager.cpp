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

ShaderManager::ShaderManager() : m_active_shader(nullptr), m_shaders() {}

ShaderManager::~ShaderManager() {
    if (m_shaders.size() > 0) {
        LogDebug(sTag, "Shaders not deleted");
    }
}

Shader* ShaderManager::LoadFromFile(const String& basename) {
    Shader* new_shader = GetShader(basename);
    if (new_shader != nullptr) {
        return new_shader;
    }

    new_shader = CreateShader();

    FileSystem& fs = FileSystem::GetInstance();

    String shader_folder = fs.Join(sRootShaderFolder, GetShaderFolder());
    String shader_descriptor_folder =
        fs.Join(sRootShaderFolder, sShaderDescriptorFolder);

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

        String filename = fs.Join(shader_folder, basename + shader_extension);

        bool filename_exist = fs.FileExists(filename);

        // Vertex and Fragment shaders are completly required
        if (!filename_exist && (shader_type == ShaderType::VERTEX ||
                                shader_type == ShaderType::FRAGMENT)) {
            LogError(sTag, "Could not find file: {}"_format(filename));
            ok = false;
        }

        if (filename_exist) {
            std::vector<byte> filename_data;
            fs.LoadFileData(filename, &filename_data);
            if (!new_shader->LoadFromMemory(
                    filename_data.data(), filename_data.size(), shader_type)) {
                LogError(sTag, "Could not load shader: {}"_format(basename));
                ok = false;
            }
        }

        if (!ok) {
            DeleteShader(new_shader);
            return nullptr;
        }
    }

    String filename = fs.Join(shader_descriptor_folder, basename + ".json");
    std::vector<byte> json_data;
    fs.LoadFileData(filename, &json_data);
    bool is_valid_descriptor = json::accept(json_data.begin(), json_data.end());
    if (is_valid_descriptor) {
        json j = json::parse(json_data.begin(), json_data.end());
        new_shader->SetDescriptor(std::move(j));
    } else {
        LogError(sTag, "Could not load shader descriptor: {}"_format(basename));
        DeleteShader(new_shader);
        return nullptr;
    }

    if (new_shader != nullptr) {
        m_shaders[basename] = new_shader;
    }

    if (m_active_shader == nullptr) {
        m_active_shader = new_shader;
    }

    return new_shader;
}

Shader* ShaderManager::LoadFromMemory(
    const String& name, std::map<ShaderType, String*> shader_data) {
    if (GetShader(name) != nullptr) {
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

    Shader* new_shader = CreateShader();

    for (auto& shader_data_pair : shader_data) {
        bool ok = true;

        ShaderType shader_type = shader_data_pair.first;
        String* shader_source = shader_data_pair.second;

        auto shader_source_data = shader_source->ToUtf8();
        if (!new_shader->LoadFromMemory(
                reinterpret_cast<const byte*>(shader_source_data.data()),
                shader_source_data.size(), shader_type)) {
            LogDebug(sTag, "Could not load shader: {}"_format(
                               static_cast<int>(shader_type)));
            ok = false;
        }

        if (!ok) {
            DeleteShader(new_shader);
            return nullptr;
        }
    }

    m_shaders[name] = new_shader;
    return new_shader;
}

Shader* ShaderManager::GetShader(const String& name) {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second : nullptr;
}

void ShaderManager::SetActiveShader(const String& name) {
    Shader* found_shader = GetShader(name);
    if (found_shader != nullptr) {
        m_active_shader = found_shader;
        UseShader(m_active_shader);
    } else {
        LogError(sTag, "Could not find a Shader named: {}"_format(name));
    }
}

Shader* ShaderManager::GetActiveShader() {
    return m_active_shader;
}

}  // namespace engine
