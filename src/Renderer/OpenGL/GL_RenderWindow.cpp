#include <Graphics/3D/Camera.hpp>
#include <Renderer/Drawable.hpp>
#include <System/LogManager.hpp>
#include <System/StringFormat.hpp>

#include "GL_RenderWindow.hpp"
#include "GL_Shader.hpp"
#include "GL_ShaderManager.hpp"
#include "GL_Utilities.hpp"

namespace engine {

namespace {

const String sTag("GL_RenderWindow");

std::vector<const char*> sRequiredExtensions = {
#ifdef OPENGL_USE_GL
    {"GL_ARB_separate_shader_objects", "GL_ARB_shading_language_420pack"}
#endif
};

}  // namespace

GL_RenderWindow::GL_RenderWindow() : m_window(nullptr), m_context(nullptr) {}

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    math::ivec2 initial_pos(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    Uint32 window_flags =
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    m_window = SDL_CreateWindow(name.GetData(), initial_pos.x, initial_pos.y,
                                size.x, size.y, window_flags);
    if (!m_window) {
        LogError(sTag, SDL_GetError());
        return false;
    }

    // Update the base class attributes
    SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
    m_name = name;

    m_context = SDL_GL_CreateContext(m_window);
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

    const char* opengl_vendor =
        reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* opengl_renderer =
        reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* opengl_version =
        reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glsl_version =
        reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

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
        const char* extension =
            reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        opengl_available_extensions.push_back(extension);
        LogInfo(sTag, String("\t") + extension);
    }

    // Check that all the required extensions are available
    bool all_extensions_found = true;
    for (auto it = std::begin(sRequiredExtensions);
         it != std::end(sRequiredExtensions); it++) {
        bool found = false;
        const char* required_extension = *it;
        for (const char* extension : opengl_available_extensions) {
            if (std::strcmp(required_extension, extension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LogError(sTag,
                     "Extension '{}' not available"_format(required_extension));
            all_extensions_found = false;
        }
    }
    if (!all_extensions_found) {
        return false;
    }

    // TODO: User enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));

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
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void GL_RenderWindow::Reposition(int left, int top) {
    if (m_window) {
        // TODO check errors
        SDL_SetWindowPosition(m_window, left, top);
    }
}

void GL_RenderWindow::Resize(int width, int height) {
    // TODO check errors
    if (m_window && !IsFullScreen()) {
        SDL_SetWindowSize(m_window, width, height);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void GL_RenderWindow::SetFullScreen(bool fullscreen, bool is_fake) {
    // TODO check errors
    if (m_window) {
        m_is_fullscreen = fullscreen;
        Uint32 flag = 0;
        if (fullscreen) {
            flag = (is_fake) ? SDL_WINDOW_FULLSCREEN_DESKTOP
                             : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(m_window, flag);

        // Update the base class attributes
        // TMP Update the ViewPort
        SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
        GL_CALL(glViewport(0, 0, m_size.x, m_size.y));
    }
}

void GL_RenderWindow::SetVSyncEnabled(bool vsync) {
    if (SDL_GL_SetSwapInterval(vsync ? 1 : 0) == 0) {
        m_is_vsync_enable = vsync;
    } else {
        m_is_vsync_enable = false;
    }
}

void GL_RenderWindow::SwapBuffers() {
    SDL_GL_SwapWindow(m_window);
    // RenderWindow::SwapBuffers();
}

void GL_RenderWindow::Clear(const Color& color) {  // RenderTarget
    GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

bool GL_RenderWindow::IsVisible() {
    Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED;
    Uint32 mask = SDL_GetWindowFlags(m_window);
    return (mask & flags) == 0;
}

void GL_RenderWindow::Draw(Drawable& drawable) {
    GL_Shader* shader = GL_ShaderManager::GetInstance().GetActiveShader();

    // Create all the MVP matrices as Identity matrices
    math::mat4 model_matrix = math::mat4();
    math::mat4 view_matrix = math::mat4();
    math::mat4 projection_matrix = math::mat4();
    math::vec3 front_vector;

    // TMP: Move this to other part
    math::vec3 light_position(3.0f, 3.0f, 3.0f);

    // TODO: Set Model position
    model_matrix *= math::Translate(math::vec3(0.0f, 0.0f, 0.0f));
    // TODO: Normalize model sizes
    model_matrix *= math::Scale(math::vec3(0.05f));

    if (m_active_camera != nullptr) {
        view_matrix = m_active_camera->GetViewMatrix();
        front_vector = m_active_camera->GetFrontVector();
    }

    float fov = math::Radians(45.f);
    float aspect_ratio = m_size.x / static_cast<float>(m_size.y);
    float z_near = 0.1f;
    float z_far = 100.0f;
    projection_matrix = math::Perspective(fov, aspect_ratio, z_near, z_far);

    if (shader != nullptr) {
        math::mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;
        math::mat4 normal_matrix = model_matrix.Inverse().Transpose();

        UniformBufferObject ubo;
        ubo.model = model_matrix;
        ubo.normalMatrix = normal_matrix;
        ubo.mvp = mvp_matrix;
        ubo.cameraFront = front_vector;
        ubo.lightPosition = light_position;
        shader->SetUniformBufferObject(ubo);
    }

    RenderWindow::Draw(drawable);  // This calls drawable.Draw(*this);
}

}  // namespace engine
