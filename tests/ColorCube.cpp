#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Input/InputManager.hpp>
#include <Graphics/Color32.hpp>
#include <Graphics/3D/Camera.hpp>
#include <System/Stopwatch.hpp>

using namespace engine;

String vertex_shader = R"(
#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec3 fragment_position;
out vec4 fragment_color;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0);

    fragment_color = color;
    fragment_position = position;
}
)";

String fragment_shader = R"(
#version 330

in vec4 fragment_color;

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

    math::mat4 Identity(1.f, 0.f, 0.f, 0.f,
                        0.f, 1.f, 0.f, 0.f,
                        0.f, 0.f, 1.f, 0.f,
                        0.f, 0.f, 0.f, 1.f);

    math::ivec2 window_size = { 800, 600 };

    bool ok = render.Initialize("My Game Test", window_size);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init failed, exiting!");
        return 1;
    }

    input.Initialize();

    math::mat4 Projection = math::Perspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    math::mat4 View;
    math::mat4 Model;

    Shader shader;
    shader.LoadFromMemory(vertex_shader, fragment_shader);
    shader.Use();

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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
        { math::vec3(1, -1, -1 ), Color32( 255, 60, 0, 255 ) },
        { math::vec3(1, 1, -1 ), Color32( 255, 60, 0, 255 ) },
        { math::vec3(-1, 1, -1 ), Color32( 255, 60, 0, 255 ) },
        { math::vec3(1, -1, -1 ), Color32( 255, 60, 0, 255 ) },
        { math::vec3(-1, -1, -1 ), Color32( 255, 60, 0, 255 ) },
        { math::vec3(-1, 1, -1 ), Color32( 255, 60, 0, 255 ) }
    };

    float teemo = 0.001f;

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), vertex_buffer_data.data(), GL_STATIC_DRAW);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(Vertex),                  // stride
        reinterpret_cast<void*>(offsetof(Vertex, position))  // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,                                // size
        GL_UNSIGNED_BYTE,                         // type
        GL_TRUE,                         // normalized?
        sizeof(Vertex),                                // stride
        reinterpret_cast<void*>(offsetof(Vertex, color))   // array buffer offset
    );

    Camera camera({ 5, -5, -5 });
    camera.LookAt({ 0, 0, 0 });

    Stopwatch timer;
    float delta_time;

    float angle = 0;

    timer.Start();
    while (!input.exit_requested()) {
        delta_time = timer.GetElapsedTime().AsSeconds();
        timer.Restart();

        render.ClearFrameBuffer(Color32::BLACK);

        auto window_size = render.window_size();

        float speed = 2.5f * delta_time;
        if (input.GetButton(SDLK_w).IsDown())
            camera.Move(speed * math::vec3(1, 0, 0));
        if (input.GetButton(SDLK_s).IsDown())
            camera.Move(speed * math::vec3(-1, 0, 0));
        if (input.GetButton(SDLK_d).IsDown())
            camera.Move(speed * math::vec3(0, 0, 1));
        if (input.GetButton(SDLK_a).IsDown())
            camera.Move(speed * math::vec3(0, 0, -1));

        if (input.GetButton(SDLK_UP).IsDown())
            camera.Move(speed * math::vec3(0, 1, 0));
        if (input.GetButton(SDLK_DOWN).IsDown())
            camera.Move(speed * math::vec3(0, -1, 0));

        camera.LookAt({ 0, 0, 0 });

        auto pos = camera.GetPosition();
        printf("%.2f %.2f %.2f\n", pos.x, pos.y, pos.z);

        float aspect_ratio = window_size.x / static_cast<float>(window_size.y);
        angle += math::Radians(delta_time * 90.f);

        // Model = math::Rotate(angle, { 1.0f, 0.3f, 0.5f });
        View = camera.GetViewMatrix();
        Projection = math::Perspective(45.0f, aspect_ratio, 0.1f, 100.0f);

        shader.SetUniform("MVP", Projection * View * Model);

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles

        input.AdvanceFrame(&render.window_size());
        render.AdvanceFrame(input.minimized());
        // render.DepthTest(false);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    return 0;
}
