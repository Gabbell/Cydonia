#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoords;

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) flat out uint outInstanceIndex;

// =================================================================================================
void main()
{
   outUV            = inTexCoords.xy;
   outInstanceIndex = gl_InstanceIndex;

   gl_Position = vec4( inPosition, 1.0 );
}
