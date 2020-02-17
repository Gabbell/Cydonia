#version 450
#extension GL_ARB_separate_shader_objects : enable

// Shader control values
// =================================================================================================
layout(set = 1, binding = 0) uniform Beta
{
    vec4 control;
};

// Material properties
// =================================================================================================
layout(set = 2, binding = 0) uniform Gamma
{
    vec4 mat;
};
layout(set = 2, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture( texSampler, inTexCoord );
}

