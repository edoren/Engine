#include "GL_RendererFactory.hpp"
#include "GL_Mesh.hpp"
#include "GL_Shader.hpp"
#include "GL_Texture2D.hpp"

namespace engine {

GL_RendererFactory::GL_RendererFactory() {}

GL_RendererFactory::~GL_RendererFactory() {}

Shader* GL_RendererFactory::CreateShader() {
    return new GL_Shader();
}

Texture2D* GL_RendererFactory::CreateTexture2D() {
    return new GL_Texture2D();
}

Mesh* GL_RendererFactory::CreateMesh() {
    return new GL_Mesh();
}

}  // namespace engine
