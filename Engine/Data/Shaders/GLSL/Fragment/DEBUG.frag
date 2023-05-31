#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( set = 0, binding = 1 ) uniform DebugParams
{
	vec4 color;
};

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() { outColor = color; }
