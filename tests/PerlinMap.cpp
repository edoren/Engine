
#include <Core/Main.hpp>
#include <Graphics/3D/Camera.hpp>
#include <Input/InputManager.hpp>
#include <Math/Noise/PerlinNoise.hpp>
#include <Math/Noise/RidgedMultiNoise.hpp>
#include <Renderer/RenderWindow.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/ShaderManager.hpp>
#include <Renderer/Texture2D.hpp>
#include <Renderer/TextureManager.hpp>

#include <Renderer/OpenGL/GL_Plugin.hpp>
#include <Renderer/OpenGL/GL_Renderer.hpp>
#include <Renderer/OpenGL/GL_Shader.hpp>

using namespace engine;

// Shader sources
String vertex_shader = R"(
#version 330 core
layout (location = 2) in vec2 texcoord;
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
out vec3 Color;
out vec2 Texcoord;
void main() {
    Color = color;
    Texcoord = texcoord;
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

String fragment_shader = R"(
#version 330 core
in vec3 Color;
in vec2 Texcoord;
out vec4 outColor;
uniform sampler2D tex;
void main() {
    outColor = texture(tex, Texcoord) * vec4(Color, 1.0);
}
)";

struct TerrainTypes {
    String name;
    float height;
    Color32 color;
};

std::vector<float> CreateNoiseMap(const math::ivec2& size,
                                  const math::vec2& offset, float scale,
                                  const math::BaseNoise& generator) {
    std::vector<float> noise_map(size.x * size.y);

    // Create the noise map
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            float pos_x = (static_cast<float>(x) + offset.x) /
                          static_cast<float>(size.x) * scale;
            float pos_y = (static_cast<float>(y) + offset.y) /
                          static_cast<float>(size.y) * scale;

            float n = (generator.GetValue(pos_x, pos_y, 1.0f) + 1.0f) / 2.0f;
            noise_map[y * size.x + x] = n;
        }
    }

    return noise_map;
}

std::vector<Color32> CreateColorMap(const math::ivec2& size,
                                    const std::vector<float>& noise_map,
                                    const std::vector<TerrainTypes>& terrains) {
    std::vector<Color32> color_map(size.x * size.y);

    // Create the color map
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++) {
            float n = noise_map[y * size.x + x];
            for (auto terrain : terrains) {
                if (n <= terrain.height) {
                    color_map[y * size.x + x] = terrain.color;
                    break;
                }
            }
        }
    }

    return color_map;
}

int main(int argc, char* argv[]) {
    Main engine(argc, argv);

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

    math::ivec2 window_size(800, 600);
    RenderWindow& window = render.GetRenderWindow();

    bool ok = window.Create("My Game Test", window_size);
    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init failed, exiting!");
        return 1;
    }

    shader_manager.LoadFromMemory("shader",
                                  {{ShaderType::eVertex, &vertex_shader},
                                   {ShaderType::eFragment, &fragment_shader}});
    shader_manager.SetActiveShader("shader");

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
        //  Position      Color             Texcoords
        -1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  // Top-left
        1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // Top-right
        1.0f,  -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  // Bottom-right
        -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f   // Bottom-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an element array
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {0, 1, 2, 2, 3, 0};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements,
                 GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat),
                          (void*)(2 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat),
                          (void*)(5 * sizeof(GLfloat)));

    std::vector<TerrainTypes> terrains({
        {"Water Deep", 0.3f, Color32(50, 99, 195, 255)},
        {"Water Shallow", 0.4f, Color32(54, 103, 199, 255)},
        {"Sand", 0.45f, Color32(210, 208, 125, 255)},
        {"Grass", 0.55f, Color32(86, 152, 23, 255)},
        {"Grass 2", 0.6f, Color32(69, 120, 20, 255)},
        {"Rock", 0.7f, Color32(90, 69, 60, 255)},
        {"Rock2", 0.9f, Color32(75, 60, 53, 255)},
        {"Snow", 1.0f, Color32::WHITE},
    });

    std::default_random_engine rand_engine;
    std::uniform_int_distribution<int> dist;
    int seed = dist(rand_engine);
    float scale = 1.5f;

    math::ivec2 image_size(128, 128);
    math::vec2 offset(1.0f, 1.0f);

    auto noise = math::PerlinNoise(seed);

    std::vector<float> noise_map =
        CreateNoiseMap(image_size, offset, scale, noise);
    std::vector<Color32> color_map =
        CreateColorMap(image_size, noise_map, terrains);

    Button& random_seed_btn = input.GetButton(SDLK_r);

    Button& move_up_btn = input.GetButton(SDLK_UP);
    Button& move_down_btn = input.GetButton(SDLK_DOWN);
    Button& move_left_btn = input.GetButton(SDLK_LEFT);
    Button& move_right_btn = input.GetButton(SDLK_RIGHT);

    // Load the image
    Image img;
    img.LoadFromMemory(color_map.data(), image_size.x, image_size.y);

    // Create the texture
    texture_manager.LoadFromImage("map", img);
    texture_manager.SetActiveTexture2D("map");

    bool update_map = false;

    while (!input.exit_requested()) {
        if (random_seed_btn.WentDown()) {
            seed = dist(rand_engine);
            noise = math::PerlinNoise(seed);
            update_map = true;
        }

        if (move_up_btn.IsDown()) {
            offset.y -= 1.0f;
            update_map = true;
        }
        if (move_down_btn.IsDown()) {
            offset.y += 1.0f;
            update_map = true;
        }
        if (move_left_btn.IsDown()) {
            offset.x -= 1.0f;
            update_map = true;
        }
        if (move_right_btn.IsDown()) {
            offset.x += 1.0f;
            update_map = true;
        }

        if (update_map) {
            noise_map = CreateNoiseMap(image_size, offset, scale, noise);
            color_map = CreateColorMap(image_size, noise_map, terrains);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_size.x, image_size.y,
                            GL_RGBA, GL_UNSIGNED_BYTE, color_map.data());

            printf("[%.2f, %.2f]\n", offset.x, offset.y);
            update_map = false;
        }

        // Clear the screen to black
        window.Clear(Color::BLACK);

        // Draw a rectangle from the 2 triangles using 6 indices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        input.AdvanceFrame();
        render.AdvanceFrame();
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);

    return 0;
}
