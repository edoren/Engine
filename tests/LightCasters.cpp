#include <Core/Main.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/Model.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/Texture2D.hpp>
#include <Renderer/TextureManager.hpp>
#include <System/Stopwatch.hpp>

#include <Renderer/OpenGL/GL_Plugin.hpp>
#include <Renderer/OpenGL/GL_Renderer.hpp>
#include <Renderer/OpenGL/GL_Shader.hpp>

using namespace engine;

String vertex_shader = R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec4 color;

out vec2 TexCoords;
out vec3 FragPosition;
out vec3 Normal;

uniform mat4 NormalMatrix;
uniform mat4 Model;
uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0f);
    FragPosition = (Model * vec4(position, 1.0f)).xyz;
    Normal = mat3(NormalMatrix) * normal;
    TexCoords = texCoords;
}
)";

String fragment_shader = R"(
#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

struct Light {
    vec3 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;
in vec3 FragPosition;
in vec3 Normal;

out vec4 Color;

uniform vec3 ViewPosition;
uniform Material material;
uniform Light light;

void main() {
    // Ambient lighting
    vec4 ambient = vec4(texture(material.diffuse, TexCoords)) * light.ambient;

    // Diffuse lighting
    vec3 normal = normalize(Normal);
    // vec3 light_direction = normalize(-light.direction);
    vec3 light_direction = normalize(light.position - FragPosition);
    float diff = max(dot(normal, light_direction), 0.0f);
    vec4 diffuse = texture(material.diffuse, TexCoords) * diff * light.diffuse;

    // Specular lighting
    vec3 view_direction = normalize(ViewPosition - FragPosition);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess);
    vec4 specular = texture(material.specular, TexCoords) * spec * light.specular;

    // Emission
    vec4 emission = texture(material.emission, TexCoords);

    Color = ambient + diffuse + specular + emission;
}
)";

String light_vertex_shader = R"(
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0f);
}
)";

String light_fragment_shader = R"(
#version 330 core

out vec4 color;

void main() {
    color = vec4(1.0f);
}
)";

String character_vertex_shader = R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec4 color;

out vec2 TexCoords;
out vec3 FragPosition;
out vec3 Normal;

uniform mat4 NormalMatrix;
uniform mat4 Model;
uniform mat4 MVP;

void main() {
    FragPosition = (Model * vec4(position, 1.0f)).xyz;
    Normal = mat3(NormalMatrix) * normal;
    TexCoords = texCoords;
    gl_Position = MVP * vec4(position, 1.0f);
}
)";

String character_fragment_shader = R"(
#version 330 core

in vec2 TexCoords;
in vec3 FragPosition;
in vec3 Normal;

out vec4 FragColor;

struct Material {
    sampler2D tex_diffuse1;
    sampler2D tex_specular1;
    sampler2D tex_normal1;
    float shininess;
};

struct Light {
    vec3 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform Material material;
uniform Light light;

void main()
{
    // Ambient lighting
    vec4 ambient = vec4(texture(material.tex_diffuse1, TexCoords)) * light.ambient;

    // Diffuse lighting
    vec3 normal = normalize(Normal);
    // vec3 light_direction = normalize(-light.direction);
    vec3 light_direction = normalize(light.position - FragPosition);
    float diff = max(dot(normal, light_direction), 0.0f);
    vec4 diffuse = texture(material.tex_diffuse1, TexCoords) * diff * light.diffuse;

    FragColor = ambient + diffuse;
    FragColor = texture(material.tex_diffuse1, TexCoords);
}
)";
#include <iostream>
int main(int argc, char* argv[]) {
    Main engine(argc, argv);

// GL_Plugin opengl;
// engine.InstallPlugin(&opengl);

// GL_Renderer opengl;
// engine.AddRenderer(&opengl);

#ifdef ENGINE_DEBUG
    engine.LoadPlugin("opengl-plugin-d");
#else
    engine.LoadPlugin("opengl-plugin");
#endif

    engine.Initialize();

    InputManager& input = InputManager::GetInstance();
    ShaderManager& shader_manager = ShaderManager::GetInstance();
    TextureManager& texture_manager = TextureManager::GetInstance();
    Renderer& render = engine.GetActiveRenderer();

    {
        math::ivec2 window_size = {800, 600};
        RenderWindow& window = render.GetRenderWindow();

        bool ok = window.Create("My Game Test", window_size);

        if (!ok) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init failed, exiting!");
            return 1;
        }

        math::mat4 ProjectionMatrix =
            math::Perspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
        math::mat4 ViewMatrix;
        math::mat4 ModelMatrix;

        Texture2D* texture = texture_manager.LoadFromFile("container2.png");
        Texture2D* specularTexture =
            texture_manager.LoadFromFile("container2_specular.png");
        // Texture2D* specularTexture =
        //     texture_manager.LoadFromFile("lighting_maps_specular_color.png");
        // Texture2D* emissionTexture =
        //     texture_manager.LoadFromFile("matrix.png");

        GL_Shader* cube_shader =
            static_cast<GL_Shader*>(shader_manager.LoadFromMemory(
                "cube_shader",
                {{ShaderType::eVertex, &vertex_shader},
                 {ShaderType::eFragment, &fragment_shader}}));
        GL_Shader* light_shader =
            static_cast<GL_Shader*>(shader_manager.LoadFromMemory(
                "light_shader",
                {{ShaderType::eVertex, &light_vertex_shader},
                 {ShaderType::eFragment, &light_fragment_shader}}));

        GL_Shader* character_shader =
            static_cast<GL_Shader*>(shader_manager.LoadFromMemory(
                "character_shader",
                {{ShaderType::eVertex, &character_vertex_shader},
                 {ShaderType::eFragment, &character_fragment_shader}}));

        // Model character("CookKirby/DolCook.obj");
        Model character("LinkOcarina/YoungLinkEquipped.obj");

        std::vector<Vertex> vertex_buffer_data = {
            // Red face
            {{1, -1, 1}, {0, 0, 1}, {1, 1}, {0, 0, 0, 0}},
            {{1, 1, 1}, {0, 0, 1}, {1, 0}, {0, 0, 0, 0}},
            {{-1, 1, 1}, {0, 0, 1}, {0, 0}, {0, 0, 0, 0}},
            {{1, -1, 1}, {0, 0, 1}, {1, 1}, {0, 0, 0, 0}},
            {{-1, -1, 1}, {0, 0, 1}, {0, 1}, {0, 0, 0, 0}},
            {{-1, 1, 1}, {0, 0, 1}, {0, 0}, {0, 0, 0, 0}},
            // Green face
            {{1, -1, 1}, {1, 0, 0}, {0, 1}, {0, 0, 0, 0}},
            {{1, 1, 1}, {1, 0, 0}, {0, 0}, {0, 0, 0, 0}},
            {{1, 1, -1}, {1, 0, 0}, {1, 0}, {0, 0, 0, 0}},
            {{1, -1, 1}, {1, 0, 0}, {0, 1}, {0, 0, 0, 0}},
            {{1, -1, -1}, {1, 0, 0}, {1, 1}, {0, 0, 0, 0}},
            {{1, 1, -1}, {1, 0, 0}, {1, 0}, {0, 0, 0, 0}},
            // Yellow face
            {{1, 1, 1}, {0, 1, 0}, {1, 0}, {0, 0, 0, 0}},
            {{-1, 1, 1}, {0, 1, 0}, {1, 1}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {0, 1, 0}, {0, 1}, {0, 0, 0, 0}},
            {{1, 1, 1}, {0, 1, 0}, {1, 0}, {0, 0, 0, 0}},
            {{1, 1, -1}, {0, 1, 0}, {0, 0}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {0, 1, 0}, {0, 1}, {0, 0, 0, 0}},
            // White face
            {{1, -1, 1}, {0, -1, 0}, {1, 0}, {0, 0, 0, 0}},
            {{-1, -1, 1}, {0, -1, 0}, {1, 1}, {0, 0, 0, 0}},
            {{-1, -1, -1}, {0, -1, 0}, {0, 1}, {0, 0, 0, 0}},
            {{1, -1, 1}, {0, -1, 0}, {1, 0}, {0, 0, 0, 0}},
            {{1, -1, -1}, {0, -1, 0}, {0, 0}, {0, 0, 0, 0}},
            {{-1, -1, -1}, {0, -1, 0}, {0, 1}, {0, 0, 0, 0}},
            // Blue face
            {{-1, -1, 1}, {-1, 0, 0}, {0, 1}, {0, 0, 0, 0}},
            {{-1, 1, 1}, {-1, 0, 0}, {0, 0}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {-1, 0, 0}, {1, 0}, {0, 0, 0, 0}},
            {{-1, -1, 1}, {-1, 0, 0}, {0, 1}, {0, 0, 0, 0}},
            {{-1, -1, -1}, {-1, 0, 0}, {1, 1}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {-1, 0, 0}, {1, 0}, {0, 0, 0, 0}},
            // Orange face
            {{1, -1, -1}, {0, 0, -1}, {1, 1}, {0, 0, 0, 0}},
            {{1, 1, -1}, {0, 0, -1}, {1, 0}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {0, 0, -1}, {0, 0}, {0, 0, 0, 0}},
            {{1, -1, -1}, {0, 0, -1}, {1, 1}, {0, 0, 0, 0}},
            {{-1, -1, -1}, {0, 0, -1}, {0, 1}, {0, 0, 0, 0}},
            {{-1, 1, -1}, {0, 0, -1}, {0, 0}, {0, 0, 0, 0}},
        };

        std::vector<math::vec3> cube_positions = {
            {0.0f, 0.0f, 0.0f},     {2.0f, 5.0f, -15.0f}, {-1.5f, -2.2f, -2.5f},
            {-3.8f, -2.0f, -12.3f}, {2.4f, -0.4f, -3.5f}, {-1.7f, 3.0f, -7.5f},
            {1.3f, -2.0f, -2.5f},   {1.5f, 2.0f, -2.5f},  {1.5f, 0.2f, -1.5f},
            {-1.3f, 1.0f, -1.5f}};

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     vertex_buffer_data.size() * sizeof(Vertex),
                     vertex_buffer_data.data(), GL_STATIC_DRAW);

        GLuint cube_VAO;
        glGenVertexArrays(1, &cube_VAO);
        glBindVertexArray(cube_VAO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,               // size
            GL_FLOAT,        // type
            GL_FALSE,        // normalized?
            sizeof(Vertex),  // stride
            reinterpret_cast<void*>(offsetof(Vertex, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,               // size
            GL_FLOAT,        // type
            GL_TRUE,         // normalized?
            sizeof(Vertex),  // stride
            reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            2,               // size
            GL_FLOAT,        // type
            GL_FALSE,        // normalized?
            sizeof(Vertex),  // stride
            reinterpret_cast<void*>(offsetof(Vertex, tex_coords)));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,
                              4,               // size
                              GL_FLOAT,        // type
                              GL_FALSE,        // normalized?
                              sizeof(Vertex),  // stride
                              reinterpret_cast<void*>(offsetof(Vertex, color)));

        glBindVertexArray(0);

        GLuint light_VAO;
        glGenVertexArrays(1, &light_VAO);
        glBindVertexArray(light_VAO);

        // position attribute buffer : vertice
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,               // size
            GL_FLOAT,        // type
            GL_FALSE,        // normalized?
            sizeof(Vertex),  // stride
            reinterpret_cast<void*>(offsetof(Vertex, position)));

        glBindVertexArray(0);

        Mouse& mouse = input.GetMouse();
        mouse.SetRelativeMouseMode(true);
        mouse.HideCursor();

        Camera camera({5, 0, 0});
        camera.LookAt({0, 0, 0});
        float camera_speed = 2.5f;
        float mouse_sensivity = 0.1f;

        Stopwatch timer;
        float delta_time;

        float angle = 0;

        math::vec4 light_color(Color::WHITE.r, Color::WHITE.g, Color::WHITE.b,
                               Color::WHITE.a);
        math::vec3 light_position(2.0f, 2.0f, 2.0f);

        timer.Start();
        while (!input.exit_requested()) {
            delta_time = timer.GetElapsedTime().AsSeconds();
            timer.Restart();

            window.Clear(Color::BLACK);

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

            ViewMatrix = camera.GetViewMatrix();

            window_size = window.GetSize();

            float aspect_ratio =
                window_size.x / static_cast<float>(window_size.y);
            angle += math::Radians(delta_time * 90.f);

            ViewMatrix = camera.GetViewMatrix();
            ProjectionMatrix = math::Perspective(math::Radians(45.f),
                                                 aspect_ratio, 0.1f, 100.0f);

            // Draw cube
            ModelMatrix = math::mat4();

            auto lol = math::Rotate(angle, {0.0f, 1.0f, 0.0f}) *
                       math::vec4(light_position, 1.0f);

            shader_manager.SetActiveShader("cube_shader");
            cube_shader->SetUniform("ViewPosition", camera.GetPosition());

            cube_shader->SetUniform("light.ambient", light_color * 0.2f);
            cube_shader->SetUniform("light.diffuse", light_color * 0.7f);
            cube_shader->SetUniform("light.specular", light_color * 1.0f);
            cube_shader->SetUniform("light.position", lol.xyz());

            cube_shader->SetUniform("material.diffuse", 0);
            cube_shader->SetUniform("material.specular", 1);
            cube_shader->SetUniform("material.emission", 2);
            cube_shader->SetUniform("material.shininess",
                                    128.0f * 0.25f / 2 / 2);

            glActiveTexture(GL_TEXTURE0);
            texture->Use();
            glActiveTexture(GL_TEXTURE1);
            specularTexture->Use();
            // glActiveTexture(GL_TEXTURE2);
            // emissionTexture->Use();

            glBindVertexArray(cube_VAO);
            for (std::size_t i = 1; i < cube_positions.size(); i++) {
                // Calculate the ModelMatrix matrix for each object and pass it
                // to
                // shader before drawing
                ModelMatrix = math::Translate(cube_positions[i] * 2);
                ModelMatrix *=
                    math::Rotate(math::Radians(10.f * i), {1.0f, 0.3f, 0.5f});

                cube_shader->SetUniform("Model", ModelMatrix);
                cube_shader->SetUniform("NormalMatrix",
                                        ModelMatrix.Inverse().Transpose());
                cube_shader->SetUniform(
                    "MVP", ProjectionMatrix * ViewMatrix * ModelMatrix);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);

            shader_manager.SetActiveShader("character_shader");

            character_shader->SetUniform("light.ambient", light_color * 0.2f);
            character_shader->SetUniform("light.diffuse", light_color * 0.7f);
            // character_shader->SetUniform("light.specular", light_color *
            // 1.0f);
            character_shader->SetUniform("light.position", lol.xyz());

            ModelMatrix = math::mat4();
            ModelMatrix *= math::Translate(math::vec3(
                0.0f, -1.75f,
                0.0f));  // translate it down so it's at the center of the scene
            ModelMatrix *= math::Scale(math::vec3(
                0.08f));  // it's a bit too big for our scene, so scale it down

            character_shader->SetUniform("Model", ModelMatrix);
            character_shader->SetUniform("NormalMatrix",
                                         ModelMatrix.Inverse().Transpose());
            character_shader->SetUniform(
                "MVP", ProjectionMatrix * ViewMatrix * ModelMatrix);
            window.Draw(character);

            // Draw the light cube
            ModelMatrix = math::mat4();
            ModelMatrix *= math::Rotate(angle, {0.0f, 1.0f, 0.0f});
            ModelMatrix *= math::Translate(light_position);
            ModelMatrix *=
                math::Scale(math::vec3(0.08f));  // Make it a smaller cube

            shader_manager.SetActiveShader("light_shader");
            light_shader->SetUniform(
                "MVP", ProjectionMatrix * ViewMatrix * ModelMatrix);

            glBindVertexArray(light_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);

            input.AdvanceFrame();
            render.AdvanceFrame();
            // render.DepthTest(false);
        }

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &cube_VAO);
        glDeleteVertexArrays(1, &light_VAO);
    }

    engine.Shutdown();

    return 0;
}
