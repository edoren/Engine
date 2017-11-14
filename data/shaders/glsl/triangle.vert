#version 450

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec2 outUV;
out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    gl_Position = inPosition;
    outUV = inUV;
}
