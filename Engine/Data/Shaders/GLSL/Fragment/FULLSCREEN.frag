#version 450
#extension GL_ARB_separate_shader_objects : enable

//#include "NOISE.h"

// =================================================================================================

// Inputs
layout(location = 0) in vec2 inUV;

// Outputs
layout(location = 0) out vec4 outColor;

// =================================================================================================
// Kind of just a playground
void main()
{
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}

