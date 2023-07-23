#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inTexCoords;
layout( location = 3 ) in vec4 inColor;  // Not used

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;

// =================================================================================================
void main()
{
   outUV       = inTexCoords.xy;
   gl_Position = vec4( inPosition, 1.0 );
}
