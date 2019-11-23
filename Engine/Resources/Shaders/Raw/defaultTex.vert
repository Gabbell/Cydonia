#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outTexCoord;

void main() {
    gl_Position = proj * view * model * inPosition;
    outColor = inColor;
    outTexCoord = inTexCoord;
}

