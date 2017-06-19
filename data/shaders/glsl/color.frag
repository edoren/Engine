#version 410

layout(location = 0) in vec4 fragment_color;
layout(location = 1) in vec3 fragment_position;

layout(location = 0) out vec4 color;

void main() {
    color = fragment_color;
}
