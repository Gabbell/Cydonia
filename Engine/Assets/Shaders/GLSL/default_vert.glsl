#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 proj;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inTexCoords;
layout(location = 3) in vec3 inNormals;

layout(location = 0) out vec4 outColor;

void main()
{
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    outColor = inColor;
}