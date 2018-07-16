#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Texture2D.hpp>

namespace engine {

class ENGINE_API RendererFactory {
public:
    RendererFactory();

    virtual ~RendererFactory();

    virtual Shader* CreateShader() = 0;

    virtual Texture2D* CreateTexture2D() = 0;

    virtual Mesh* CreateMesh() = 0;
};

}  // namespace engine
