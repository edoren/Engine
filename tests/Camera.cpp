#include <Renderer/Renderer.hpp>
#include <Input/InputManager.hpp>
#include <Graphics/Color32.hpp>
#include <Graphics/ResourceManager.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Graphics/Image.hpp>
#include <System/Stopwatch.hpp>

using namespace engine;

String vertex_shader = R"(
#version 330

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 vertex_color;

out vec3 fragment_position;
out vec4 fragment_color;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertex_position, 1.0);

    fragment_color = vertex_color;
    fragment_position = vertex_position;
}
)";

String fragment_shader = R"(
#version 330

in vec4 fragment_color;
in vec3 fragment_position;

out vec4 color;

void main() {
    color = fragment_color;
}
)";

struct Vertex {
    Vertex(const math::vec3& position, const Color32& color) :
            position(position),
            color(color) {
    }
    math::vec3 position;
    Color32 color;
};

int main(int argc, char* argv[]) {
    Renderer render;
    InputManager input;
    ResourceManager resources;

    math::ivec2 window_size = { 800, 600 };

    bool ok = render.Initialize("My Game Test", window_size);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init failed, exiting!");
        return 1;
    }

    input.Initialize();

    Shader shader;
    shader.LoadFromMemory(vertex_shader, fragment_shader);
    shader.Use();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    std::vector<Vertex> vertex_buffer_data{
        // Red face
        { math::vec3(1, -1, 1 ), Color32::RED },
        { math::vec3(1, 1, 1 ), Color32::RED },
        { math::vec3(-1, 1, 1 ), Color32::RED },
        { math::vec3(1, -1, 1 ), Color32::RED },
        { math::vec3(-1, -1, 1 ), Color32::RED },
        { math::vec3(-1, 1, 1 ), Color32::RED },
        // Green face
        { math::vec3(1, -1, 1 ), Color32::GREEN },
        { math::vec3(1, 1, 1 ), Color32::GREEN },
        { math::vec3(1, 1, -1 ), Color32::GREEN },
        { math::vec3(1, -1, 1 ), Color32::GREEN },
        { math::vec3(1, -1, -1 ), Color32::GREEN },
        { math::vec3(1, 1, -1 ), Color32::GREEN },
        // Yellow face
        { math::vec3(1, 1, 1 ), Color32::YELLOW },
        { math::vec3(-1, 1, 1 ), Color32::YELLOW },
        { math::vec3(-1, 1, -1 ), Color32::YELLOW },
        { math::vec3(1, 1, 1 ), Color32::YELLOW },
        { math::vec3(1, 1, -1 ), Color32::YELLOW },
        { math::vec3(-1, 1, -1 ), Color32::YELLOW },
        // White face
        { math::vec3(1, -1, 1 ), Color32::WHITE },
        { math::vec3(-1, -1, 1 ), Color32::WHITE },
        { math::vec3(-1, -1, -1 ), Color32::WHITE },
        { math::vec3(1, -1, 1 ), Color32::WHITE },
        { math::vec3(1, -1, -1 ), Color32::WHITE },
        { math::vec3(-1, -1, -1 ), Color32::WHITE },
        // Blue face
        { math::vec3(-1, -1, 1 ), Color32::BLUE },
        { math::vec3(-1, 1, 1 ), Color32::BLUE },
        { math::vec3(-1, 1, -1 ), Color32::BLUE },
        { math::vec3(-1, -1, 1 ), Color32::BLUE },
        { math::vec3(-1, -1, -1 ), Color32::BLUE },
        { math::vec3(-1, 1, -1 ), Color32::BLUE },
        // Orange face
        { math::vec3(1, -1, -1 ), Color32( 255, 165, 0, 255 ) },
        { math::vec3(1, 1, -1 ), Color32( 255, 165, 0, 255 ) },
        { math::vec3(-1, 1, -1 ), Color32( 255, 165, 0, 255 ) },
        { math::vec3(1, -1, -1 ), Color32( 255, 165, 0, 255 ) },
        { math::vec3(-1, -1, -1 ), Color32( 255, 165, 0, 255 ) },
        { math::vec3(-1, 1, -1 ), Color32( 255, 165, 0, 255 ) }
    };

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), vertex_buffer_data.data(), GL_STATIC_DRAW);

    // 1rst attribute buffer : vertices
    glVertexAttribPointer(
        0,
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(Vertex),                  // stride
        reinterpret_cast<void*>(offsetof(Vertex, position))  // array buffer offset
    );
    glEnableVertexAttribArray(0);

    // 2nd attribute buffer : colors
    glVertexAttribPointer(
        1,
        4,                                // size
        GL_UNSIGNED_BYTE,                         // type
        GL_TRUE,                         // normalized?
        sizeof(Vertex),                                // stride
        reinterpret_cast<void*>(offsetof(Vertex, color))   // array buffer offset
    );
    glEnableVertexAttribArray(1);

    Mouse& mouse = input.GetMouse();
    mouse.SetRelativeMouseMode(true);
    mouse.HideCursor();

    // Model matrix : an identity matrix (model will be at the origin)
    math::mat4 Model;
    // View matrix
    math::mat4 View;
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    math::mat4 Projection = math::Perspective(45.0f, render.window_size().x / (float)render.window_size().y, 0.1f, 100.0f);

    Stopwatch timer;
    float delta_time;

    Camera camera;
    float camera_speed = 2.5f;
    float mouse_sensivity = 0.1f;

    std::vector<math::vec3> cube_positions = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}
    };

    float angle = 0.f;

    timer.Start();
    while (!input.exit_requested()) {
        delta_time = timer.GetElapsedTime().AsSeconds();
        timer.Restart();

        render.ClearFrameBuffer(Color32::BLACK);

        // Camera movement
        const math::vec3& camera_front = camera.GetFrontVector();
        math::vec3 camera_forward(camera_front.x, 0, camera_front.z);
        camera_forward = math::Normalize(camera_forward);

        // Camera mouse movement
        math::vec2 mouse_delta(mouse.pointer.mousedelta);
        camera.Rotate(mouse_delta * mouse_sensivity);

        // Camera key movements
        float speed = camera_speed * delta_time;
        if (input.GetButton(SDLK_w).IsDown())
            camera.Move(speed * camera_forward);
        if (input.GetButton(SDLK_s).IsDown())
            camera.Move(speed * -camera_forward);
        if (input.GetButton(SDLK_d).IsDown())
            camera.Move(speed * camera.GetRightVector());
        if (input.GetButton(SDLK_a).IsDown())
            camera.Move(speed * -camera.GetRightVector());

        if (input.GetButton(SDLK_SPACE).IsDown())
            camera.Move(speed * Camera::WORLD_UP);
        if (input.GetButton(SDLK_LSHIFT).IsDown())
            camera.Move(speed * -Camera::WORLD_UP);

        View = camera.GetViewMatrix();

        shader.SetUniform("MVP", Projection * View * Model);

        angle += math::Radians(delta_time * 90.f);
        for(std::size_t  i = 0; i < cube_positions.size(); i++) {
            // Calculate the model matrix for each object and pass it to shader before drawing
            Model = math::Translate(cube_positions[i] * 2);
            Model *= math::Rotate(angle * (1 / (i+1.f)), { 1.0f, 0.3f, 0.5f });

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        input.AdvanceFrame(&render.window_size());
        render.AdvanceFrame(input.minimized());
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VertexArrayID);

    return 0;
}
