#version 130

in vec3 vertex_position;
in vec4 vertex_color;

out vec3 fragment_position;
out vec4 fragment_color;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertex_position, 1.0);

    fragment_color = vertex_color;
    fragment_position = vertex_position;
}
