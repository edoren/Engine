#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_color;

layout(location = 0) out vec4 fragment_color;
layout(location = 1) out vec3 fragment_position;

uniform Data {
    mat4 MVP;
} data;

void main() {
    gl_Position = data.MVP * vec4(vertex_position, 1.0);

    fragment_color = vertex_color;
    fragment_position = vertex_position;
}
