#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

float random( vec2 st )
{
   return fract( sin( dot( st.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453123 );
}

void main() { outColor = vec4( vec3( random( inUV ) ), 1.0 ); }