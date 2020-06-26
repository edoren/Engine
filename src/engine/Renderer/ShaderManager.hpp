#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Shader.hpp>
#include <System/String.hpp>

#include <memory>

namespace engine {

class ENGINE_API ShaderManager : public Singleton<ShaderManager> {
public:
    ShaderManager();

    virtual ~ShaderManager();

    virtual void initialize();

    virtual void shutdown();

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
    Shader* loadFromFile(const String& basename);

    ////////////////////////////////////////////////////////////
    /// @brief Load a shader from the memory
    ///
    /// @details This will search inside a std::map for each shader type
    ///
    /// @warning Vertex and Fragment shaders are required
    ///
    /// @return On success returns the Shader handler or nullptr on failure
    ////////////////////////////////////////////////////////////
    Shader* loadFromMemory(const String& name, const std::map<ShaderType, String*>& shaderDataMap);

    Shader* getShader(const String& name);

    void setActiveShader(const String& basename);

    Shader* getActiveShader();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static ShaderManager& GetInstance();

    ////////////////////////////////////////////////////////////
    /// @brief Override standard Singleton retrieval.
    ///
    /// @remarks Why do we do this? Well, it's because the Singleton
    ///          implementation is in a .hpp file, which means it gets
    ///          compiled into anybody who includes it. This is needed
    ///          for the Singleton template to work, but we actually
    ///          only compiled into the implementation of the class
    ///          based on the Singleton, not all of them. If we don't
    ///          change this, we get link errors when trying to use the
    ///          Singleton-based class from an outside dll.
    ///
    /// @par This method just delegates to the template version anyway,
    ///      but the implementation stays in this single compilation unit,
    ///      preventing link errors.
    ////////////////////////////////////////////////////////////
    static ShaderManager* GetInstancePtr();

protected:
    virtual std::unique_ptr<Shader> createShader() = 0;

    virtual void useShader(Shader* shader) = 0;

    virtual const String& getShaderFolder() const = 0;

    Shader* m_activeShader;
    std::map<String, std::unique_ptr<Shader>> m_shaders;
};

}  // namespace engine
