#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( set = 0, binding = 1 ) uniform sampler2D texSampler;

// =================================================================================================

// Inputs
layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec3 inTexCoord;

// Outputs
layout( location = 0 ) out vec4 outColor;

// =================================================================================================

void main() { outColor = texture( texSampler, inTexCoord.xy ); }
