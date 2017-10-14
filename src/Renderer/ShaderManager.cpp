#include <Renderer/ShaderManager.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

namespace engine {

namespace {

const String sTag("ShaderManager");

const String sRootShaderFolder("shaders");

}  // namespace

template <>
ShaderManager* Singleton<ShaderManager>::sInstance = nullptr;

ShaderManager& ShaderManager::GetInstance() {
    assert(sInstance);
    return (*sInstance);
}

ShaderManager* ShaderManager::GetInstancePtr() {
    return sInstance;
}

ShaderManager::ShaderManager() : m_active_shader(nullptr), m_shaders() {}

ShaderManager::~ShaderManager() {
    if (m_shaders.size() > 0) {
        LogDebug(sTag, "Shaders not deleted.");
    }
}

Shader* ShaderManager::LoadFromFile(const String& basename) {
    if (GetShader(basename) != nullptr) {
        LogError(sTag, "Shader '{}' already loaded");
        return nullptr;
    }

    Shader* new_shader = CreateShader();

    FileSystem& fs = FileSystem::GetInstance();

    String folder = fs.Join(sRootShaderFolder, GetShaderFolder());

    auto shader_types = {ShaderType::eVertex, ShaderType::eFragment,
                         ShaderType::eGeometry};
    for (auto shader_type : shader_types) {
        bool ok = true;

        const char* shader_extension = "";
        switch (shader_type) {
            case ShaderType::eVertex:
                shader_extension = ".vert";
                break;
            case ShaderType::eFragment:
                shader_extension = ".frag";
                break;
            case ShaderType::eGeometry:
                shader_extension = ".geom";
                break;
        }

        String filename = fs.Join(folder, basename + shader_extension);

        bool filename_exist = fs.FileExists(filename);

        // Vertex and Fragment shaders are completly required
        if (!filename_exist && (shader_type == ShaderType::eVertex ||
                                shader_type == ShaderType::eFragment)) {
            LogDebug(sTag, "Could not find file: {}"_format(filename.ToUtf8()));
            ok = false;
        }

        if (filename_exist) {
            std::vector<byte> filename_data;
            fs.LoadFileData(filename, &filename_data);
            if (!new_shader->LoadFromMemory(
                    filename_data.data(), filename_data.size(), shader_type)) {
                LogDebug(sTag,
                         "Could not load shader: {}"_format(basename.ToUtf8()));
                ok = false;
            }
        }

        if (!ok) {
            DeleteShader(new_shader);
            return nullptr;
        }
    }

    if (new_shader != nullptr) {
        m_shaders[basename] = new_shader;
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
    auto it_vertex = shader_data.find(ShaderType::eVertex);
    auto it_fragment = shader_data.find(ShaderType::eFragment);
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
        LogError(sTag,
                 "Could not find a Shader named: {}"_format(name.ToUtf8()));
    }
}

Shader* ShaderManager::GetActiveShader() {
    return m_active_shader;
}

}  // engine
