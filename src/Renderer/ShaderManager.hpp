#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Shader.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API ShaderManager : public Singleton<ShaderManager> {
public:
    ShaderManager();
    virtual ~ShaderManager();

    ////////////////////////////////////////////////////////////
    /// @brief Load a shader from the filesystem
    ///
    /// @details This will search for files with the next extensions:
    ///          - .vert : Vertex shader
    ///          - .frag : Fragment shader
    ///          - .geom : Geometry shader
    ///
    /// @warning Vertex and Fragment shaders are required
    ///
    /// @return On success returns the Shader handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Shader* LoadFromFile(const String& basename);

    ////////////////////////////////////////////////////////////
    /// @brief Load a shader from the memory
    ///
    /// @details This will search inside a std::map for each shader type
    ///
    /// @warning Vertex and Fragment shaders are required
    ///
    /// @return On success returns the Shader handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Shader* LoadFromMemory(const String& name,
                           std::map<ShaderType, String*> shader_data_map);

    Shader* GetShader(const String& name);

    void SetActiveShader(const String& basename);

    Shader* GetActiveShader();

protected:
    virtual Shader* CreateShader() = 0;

    virtual void DeleteShader(Shader* shader) = 0;

    virtual void SetActiveShader(Shader* shader) = 0;

    virtual const String& GetShaderFolder() const = 0;

    Shader* m_active_shader;
    std::map<String, Shader*> m_shaders;
};

}  // engine
