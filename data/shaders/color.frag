#version 130

in vec4 fragment_color;
in vec3 fragment_position;

out vec4 color;

uniform float time;

void main() {
    color = fragment_color;
}
