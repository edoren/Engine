#include <Graphics/3D/Camera.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "GL_RenderWindow.hpp"
#include "GL_Shader.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_Utilities.hpp"

namespace engine {

namespace {

const String sTag("GL_RenderWindow");

}  // namespace

GL_RenderWindow::GL_RenderWindow() : m_context(nullptr) {}

GL_RenderWindow::~GL_RenderWindow() {
    Destroy();
}

bool GL_RenderWindow::Create(const String& name, const math::ivec2& size) {
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

    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 window_flags(SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y, size.x, size.y, window_flags);
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

    const char* opengl_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* opengl_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* opengl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glsl_version = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    LogInfo(sTag, String("OpenGL Vendor: ") + opengl_vendor);
    LogInfo(sTag, String("OpenGL Renderer: ") + opengl_renderer);
    LogInfo(sTag, String("OpenGL Version: ") + opengl_version);
    LogInfo(sTag, String("GLSL Version: ") + glsl_version);

    GLint num_extensions = 0;
    GL_CALL(glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions));

    LogInfo(sTag, "OpenGL Extensions: {}"_format(num_extensions));
    std::vector<const char*> opengl_available_extensions;
    opengl_available_extensions.reserve(num_extensions);
    for (GLint i = 0; i < num_extensions; i++) {
        const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        opengl_available_extensions.push_back(extension);
        LogInfo(sTag, String("\t") + extension);
    }

    // Check that all the required extensions are available
    bool all_extensions_found = true;
    const auto& required_extensions = GL_Shader::GetRequiredExtensions();
    for (const auto* required_extension : required_extensions) {
        bool found = false;
        for (const char* extension : opengl_available_extensions) {
            if (std::strcmp(required_extension, extension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LogError(sTag, "Extension '{}' not available"_format(required_extension));
            all_extensions_found = false;
        }
    }
    if (!all_extensions_found) {
        return false;
    }

    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));

    // Update the base class attributes
    RenderWindow::Create(name, size);

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

void GL_RenderWindow::Destroy() {
    if (m_context) {
        SDL_GL_DeleteContext(m_context);
        m_context = nullptr;
    }
    RenderWindow::Destroy();
}

void GL_RenderWindow::Resize(int width, int height) {
    RenderWindow::Resize(width, height);
    // TODO: TMP Update the ViewPort
    GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
}

void GL_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    RenderWindow::SetFullScreen(fullscreen, is_fake);
    // TODO: TMP Update the ViewPort
    GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
}

void GL_RenderWindow::SetVSyncEnabled(bool vsync) {
    if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
        m_is_vsync_enable = vsync;
    } else {
        m_is_vsync_enable = false;
    }
}

void GL_RenderWindow::SwapBuffers() {
    // Update static uniform buffer
    GL_Shader* shader = GL_ShaderManager::GetInstance().GetActiveShader();
    const Camera* active_camera = GetActiveCamera();

    math::vec3 front_vector;
    math::vec3 light_position;  // TMP: Get this from other
                                //      part a LightManager maybe?
    if (active_camera != nullptr) {
        front_vector = active_camera->GetFrontVector();
        light_position = active_camera->GetPosition();
    }

    UniformBufferObject& ubo = shader->GetUBO();
    ubo.SetAttributeValue("cameraFront", front_vector);
    ubo.SetAttributeValue("lightPosition", light_position);
    ///

    SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(m_window));
}

void GL_RenderWindow::OnWindowResized(const math::ivec2& size) {
    ENGINE_UNUSED(size);
}

void GL_RenderWindow::Clear(const Color& color) {  // RenderTarget
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

}  // namespace engine
