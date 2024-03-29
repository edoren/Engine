#include <Graphics/3D/Camera.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>
#include <System/StringView.hpp>
#include <Util/Container/Vector.hpp>

#include "GL_RenderWindow.hpp"
#include "GL_Shader.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_Utilities.hpp"

namespace engine::plugin::opengl {

namespace {

const StringView sTag("GL_RenderWindow");

}  // namespace

GL_RenderWindow::GL_RenderWindow() : m_context(nullptr) {}

GL_RenderWindow::~GL_RenderWindow() {
    destroy();
}

bool GL_RenderWindow::create(const String& name, const math::ivec2& size) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if PLATFORM_TYPE_IS(PLATFORM_TYPE_DESKTOP)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    math::ivec2 initialPos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 windowFlags(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    m_window = SDL_CreateWindow(name.getData(), initialPos.x, initialPos.y, size.x, size.y, windowFlags);
    if (!m_window) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    m_context = SDL_GL_CreateContext(reinterpret_cast<SDL_Window*>(m_window));
    if (!m_context) {
        LogError(sTag, SDL_GetError());
        return false;
    }

#if PLATFORM_TYPE_IS(PLATFORM_TYPE_DESKTOP)
    // glewExperimental = GL_TRUE;
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        LogError(sTag, "GLEW initialization failed");
        return false;
    }
#endif

    const char* openglVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* openglRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* openglVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    LogInfo(sTag, String("OpenGL Vendor: ") + openglVendor);
    LogInfo(sTag, String("OpenGL Renderer: ") + openglRenderer);
    LogInfo(sTag, String("OpenGL Version: ") + openglVersion);
    LogInfo(sTag, String("GLSL Version: ") + glslVersion);

    GLint numExtensions = 0;
    GL_CALL(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));

    LogInfo(sTag, "OpenGL Extensions: {}", numExtensions);
    Vector<const char*> openglAvailableExtensions;
    openglAvailableExtensions.reserve(numExtensions);
    for (GLint i = 0; i < numExtensions; i++) {
        const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        openglAvailableExtensions.push_back(extension);
        LogInfo(sTag, String("\t") + extension);
    }

    // Check that all the required extensions are available
    bool allExtensionsFound = true;
    const auto& requiredExtensions = GL_Shader::GetRequiredExtensions();
    for (const auto* requiredExtension : requiredExtensions) {
        bool found = false;
        for (const char* extension : openglAvailableExtensions) {
            if (std::strcmp(requiredExtension, extension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LogError(sTag, "Extension '{}' not available", requiredExtension);
            allExtensionsFound = false;
        }
    }
    if (!allExtensionsFound) {
        return false;
    }

    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));

    // Update the base class attributes
    RenderWindow::create(name, size);

    return true;
}

// void GL_RenderWindow::AdvanceFrame(bool minimized) {
//    if (minimized) {
//        // Save some cpu / battery:
//        SDL_Delay(10);
//    } else {
//        SDL_GL_SwapWindow(m_window);
//    }
//    // SDL_GetWindowSize(m_window, &m_window_size.x, &m_window_size.y);
//    // GL_CALL(glViewport(0, 0, m_window_size.x, m_window_size.y));
//}

void GL_RenderWindow::destroy() {
    if (m_context) {
        SDL_GL_DeleteContext(m_context);
        m_context = nullptr;
    }
    RenderWindow::destroy();
}

void GL_RenderWindow::resize(int width, int height) {
    RenderWindow::resize(width, height);
    // TODO: TMP Update the ViewPort
    GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
}

void GL_RenderWindow::setFullScreen(bool fullscreen, bool isFake) {
    RenderWindow::setFullScreen(fullscreen, isFake);
    // TODO: TMP Update the ViewPort
    GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
}

void GL_RenderWindow::setVSyncEnabled(bool vsync) {
    if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
        m_isVsyncEnabled = vsync;
    } else {
        m_isVsyncEnabled = false;
    }
}

void GL_RenderWindow::swapBuffers() {
    // Update static uniform buffer
    GL_Shader* shader = GL_ShaderManager::GetInstance().getActiveShader();
    const Camera* activeCamera = getActiveCamera();

    math::vec3 frontVector;
    math::vec3 lightPosition;  // TMP: Get this from other
                               //      part a LightManager maybe?
    if (activeCamera != nullptr) {
        frontVector = activeCamera->getFrontVector();
        lightPosition = activeCamera->getPosition();
    }

    UniformBufferObject& ubo = shader->getUbo();
    ubo.setAttributeValue("cameraFront", frontVector);
    ubo.setAttributeValue("lightPosition", lightPosition);
    ///

    SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(m_window));
}

void GL_RenderWindow::onWindowResized(const math::ivec2& size) {
    ENGINE_UNUSED(size);
}

void GL_RenderWindow::clear(const Color& color) {  // RenderTarget
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

}  // namespace engine::plugin::opengl
