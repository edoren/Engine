#version 420 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#ifdef VULKAN
layout (set = 1, binding = 0) uniform sampler2D tex_diffuse1;
#else  // OPENGL
uniform sampler2D tex_diffuse1;
#endif

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;
layout (location = 5) in vec3 inFragPosition;

layout (location = 0) out vec4 outFragColor;

void main()
{
    vec4 color = texture(tex_diffuse1, inUV);

    // Ambient lighting
    vec4 ambient = color * vec4(0.2);

    // Diffuse lighting
    vec3 normal = normalize(inNormal);
    // vec3 light_direction = normalize(-light.direction);
    vec3 light_direction = normalize(inLightVec - inFragPosition);
    float diff = max(dot(normal, light_direction), 0.0f);
    vec4 diffuse = color * diff * vec4(0.7f);

    outFragColor = ambient + diffuse;
}
