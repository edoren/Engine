#version 420 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 normalMatrix;
    mat4 mvp;
    vec3 cameraFront;
    vec3 lightPosition;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;
layout (location = 5) out vec3 outFragPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outNormal = mat3(ubo.normalMatrix) * inNormal;
    outColor = inColor;
    outUV = inUV;
    outFragPosition = (ubo.model * vec4(inPos, 1.0f)).xyz;
    outViewVec = ubo.cameraFront;
    outLightVec = ubo.lightPosition;

    gl_Position = ubo.mvp * vec4(inPos, 1.0f);
}
