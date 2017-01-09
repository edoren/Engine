#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Input/InputManager.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/Color32.hpp>
#include <Graphics/3D/Camera.hpp>
#include <System/Stopwatch.hpp>

using namespace engine;

String vertex_shader = R"(
#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 FragPosition;
out vec3 Normal;

uniform mat4 NormalMatrix;
uniform mat4 Model;
uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0f);
    FragPosition = (Model * vec4(position, 1.0f)).xyz;
    Normal = mat3(NormalMatrix) * normal;
}
)";

String fragment_shader = R"(
#version 330

in vec3 FragPosition;
in vec3 Normal;

out vec4 Color;

uniform vec3 LightPosition;
uniform vec3 ViewPosition;
uniform vec4 ObjectColor;
uniform vec4 LightColor;

void main() {
    // Ambient lighting
    float ambient_strength = 0.2f;
    vec4 ambient = ambient_strength * LightColor;

    // Diffuse lighting
    vec3 normal = normalize(Normal);
    vec3 light_direction = normalize(LightPosition - FragPosition);
    float diff = max(dot(normal, light_direction), 0.0f);
    vec4 diffuse = diff * LightColor;

    // Specular lighting
    float shininess = 16.0f;
    float specular_strength = 0.5f;
    vec3 view_direction = normalize(ViewPosition - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), shininess);
    vec4 specular = specular_strength * spec * LightColor;

    Color = (ambient + diffuse + specular) * ObjectColor;
}
)";

String light_vertex_shader = R"(
#version 330

layout (location = 0) in vec3 position;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0f);
}
)";

String light_fragment_shader = R"(
#version 330

out vec4 color;

void main() {
    color = vec4(1.0f);
}
)";

struct Vertex {
    Vertex(const math::vec3& position, const math::vec3& normal) :
        position(position),
        normal(normal) {
    }
    math::vec3 position;
    math::vec3 normal;
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

    Shader cube_shader;
    cube_shader.LoadFromMemory(vertex_shader, fragment_shader);

    Shader light_shader;
    light_shader.LoadFromMemory(light_vertex_shader, light_fragment_shader);

    std::vector<Vertex> vertex_buffer_data {
        // Red face
        { {1, -1, 1}, {0, 0, 1} },
        { {1, 1, 1}, {0, 0, 1} },
        { {-1, 1, 1}, {0, 0, 1} },
        { {1, -1, 1}, {0, 0, 1} },
        { {-1, -1, 1}, {0, 0, 1} },
        { {-1, 1, 1}, {0, 0, 1} },
        // Green face
        { {1, -1, 1}, {1, 0, 0} },
        { {1, 1, 1}, {1, 0, 0} },
        { {1, 1, -1}, {1, 0, 0} },
        { {1, -1, 1}, {1, 0, 0} },
        { {1, -1, -1}, {1, 0, 0} },
        { {1, 1, -1}, {1, 0, 0} },
        // Yellow face
        { {1, 1, 1}, {0, 1, 0} },
        { {-1, 1, 1}, {0, 1, 0} },
        { {-1, 1, -1}, {0, 1, 0} },
        { {1, 1, 1}, {0, 1, 0} },
        { {1, 1, -1}, {0, 1, 0} },
        { {-1, 1, -1}, {0, 1, 0} },
        // White face
        { {1, -1, 1}, {0, -1, 0} },
        { {-1, -1, 1}, {0, -1, 0} },
        { {-1, -1, -1}, {0, -1, 0} },
        { {1, -1, 1}, {0, -1, 0} },
        { {1, -1, -1}, {0, -1, 0} },
        { {-1, -1, -1}, {0, -1, 0} },
        // Blue face
        { {-1, -1, 1}, {-1, 0, 0} },
        { {-1, 1, 1}, {-1, 0, 0} },
        { {-1, 1, -1}, {-1, 0, 0} },
        { {-1, -1, 1}, {-1, 0, 0} },
        { {-1, -1, -1}, {-1, 0, 0} },
        { {-1, 1, -1}, {-1, 0, 0} },
        // Orange face
        { {1, -1, -1}, {0, 0, -1} },
        { {1, 1, -1}, {0, 0, -1} },
        { {-1, 1, -1}, {0, 0, -1} },
        { {1, -1, -1}, {0, 0, -1} },
        { {-1, -1, -1}, {0, 0, -1} },
        { {-1, 1, -1}, {0, 0, -1} },
    };

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), vertex_buffer_data.data(), GL_STATIC_DRAW);

    GLuint cube_VAO;
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    glVertexAttribPointer(
        0,
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(Vertex),                  // stride
        reinterpret_cast<void*>(offsetof(Vertex, position))  // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,                  // size
        GL_FLOAT,           // type
        GL_TRUE,           // normalized?
        sizeof(Vertex),                  // stride
        reinterpret_cast<void*>(offsetof(Vertex, normal))  // array buffer offset
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    GLuint light_VAO;
    glGenVertexArrays(1, &light_VAO);
    glBindVertexArray(light_VAO);

    // position attribute buffer : vertice
    glVertexAttribPointer(
        0,
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(Vertex),                  // stride
        reinterpret_cast<void*>(offsetof(Vertex, position))  // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    Mouse& mouse = input.GetMouse();
    mouse.SetRelativeMouseMode(true);
    mouse.HideCursor();

    Camera camera({ 5, 0, 0 });
    camera.LookAt({ 0, 0, 0 });
    float camera_speed = 2.5f;
    float mouse_sensivity = 0.1f;

    Stopwatch timer;
    float delta_time;

    float angle = 0;

    auto color1 = Color::Lerp(Color::BLUE, Color::WHITE, 0.0);
    auto color2 = Color::WHITE;
    math::vec4 cube_color(color1.r, color1.g, color1.b, color1.a);
    math::vec4 light_color(color2.r, color2.g, color2.b, color2.a);

    math::vec3 light_position(2.0f, 2.0f, 2.0f);


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

        auto window_size = render.window_size();

        float aspect_ratio = window_size.x / static_cast<float>(window_size.y);
        angle += math::Radians(delta_time * 90.f);

        View = camera.GetViewMatrix();
        Projection = math::Perspective(45.0f, aspect_ratio, 0.1f, 100.0f);

        // Draw cube
        Model = math::mat4();

        auto lol = math::vec4(light_position, 1.0f) * math::Rotate(angle, { 0.0f, -1.0f, 0.0f });

        cube_shader.Use();
        cube_shader.SetUniform("Model", Model);
        cube_shader.SetUniform("NormalMatrix", Model.Inverse().Transpose());
        cube_shader.SetUniform("MVP", Projection * View * Model);
        cube_shader.SetUniform("ObjectColor", cube_color);
        cube_shader.SetUniform("LightColor", light_color);
        cube_shader.SetUniform("LightPosition", lol.xyz());
        cube_shader.SetUniform("ViewPosition", camera.GetPosition());

        glBindVertexArray(cube_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Draw the light cube
        Model = math::mat4();
        Model *= math::Rotate(angle, { 0.0f, 1.0f, 0.0f });
        Model *= math::Translate(light_position);
        Model *= math::Scale(math::vec3(0.08f)); // Make it a smaller cube

        light_shader.Use();
        light_shader.SetUniform("MVP", Projection * View * Model);

        glBindVertexArray(light_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        input.AdvanceFrame(&render.window_size());
        render.AdvanceFrame(input.minimized());
        // render.DepthTest(false);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &cube_VAO);
    glDeleteVertexArrays(1, &light_VAO);

    return 0;
}
