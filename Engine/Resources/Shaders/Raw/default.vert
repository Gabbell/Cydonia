#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
	mat4 mv;
	mat4 proj;
};

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
    gl_Position = proj * mv * inPosition;
    outColor = inColor;
}